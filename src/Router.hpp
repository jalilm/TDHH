#ifndef TDHH_ROUTER_HPP
#define TDHH_ROUTER_HPP

#include <string>
#include "PacketsReader.hpp"
#include "Hyperloglog.hpp"
#include "Heap.hpp"
#include <map>
#include <boost/functional/hash.hpp>
#include <boost/math/special_functions/beta.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "Utils.hpp"

namespace TDHH {
    using namespace std;
    using namespace hll;

    const int ITERATIONS = 1;

    class Router {

    private:
        DATASET dataset;

    protected:
        string filename;
        PacketsReader pr;

    public:
        explicit Router(const string &filename, DATASET dataset) :
        filename(filename),
        pr(filename, dataset),
        dataset(dataset)
        {}

        void reset() {
            pr.reset();
        }

        map<int, map<int,vector<double>>> volumeEstimation(vector<int> counters) {
            map<int, map<int,vector<double>>> res;
            std::random_device rd;
            map<int, vector<HyperLogLog> > counter_to_hll_arr;
            for (const int c : counters) {
                int bits = log2(c);
                vector<HyperLogLog> hll_arr;
                for (int I = 0; I < ITERATIONS; I++) {
                    HyperLogLog hll(bits);
                    hll.set_seed(rd());
                    hll_arr.push_back(hll);
                }
                counter_to_hll_arr.insert(pair<int, vector<HyperLogLog>>(c, hll_arr));
            }
            auto pkt = pr.getNextIPPacket();
            int num_pkts = 0;
            while (pkt != nullptr) {
                ++num_pkts;
                const auto& pkt_string = pkt->getReprString();
                for(const int c: counters) {
                    for (auto & hll : counter_to_hll_arr.at(c)) {
                        hll.add(pkt_string.c_str(), pkt_string.size());
                    }
                }
                delete pkt;
                if (num_pkts % 1000000 == 0) {
                    for(const int c: counters) {
                        for (const auto & hll : counter_to_hll_arr.at(c)) {
                            res[c][num_pkts].push_back(hll.estimate());
                        }
                    }
                }
                pkt = pr.getNextIPPacket();
            }
            for(const int c: counters) {
                for (const auto & hll : counter_to_hll_arr.at(c)) {
                    res[c][num_pkts].push_back(hll.estimate());
                }
            }
            return res;
        }

        Heap* sample(double eps, double delta, bool formula_chi = true) {
            unsigned int chi;
            if (formula_chi) {
                 chi = ceil(3.0 / (eps * eps) * log2(2.0 / delta));
            } else {
                chi = ceil(9.0 / (eps * eps) * log2(2.0 / (delta * eps)));
            }
            Heap* chiMax = new Heap(chi);
            auto pkt = pr.getNextIPPacket();
            while (pkt != nullptr) {
                chiMax->Add(pkt->getReprString());
                delete pkt;
                pkt = pr.getNextIPPacket();
            }
            return chiMax;
        }

        map<pair<double,double>, vector<map<string, double>>> heavy_hitters(vector<pair<double,double>> params) {
            map<pair<double,double>, vector<map<string, double>>> res;

            map<pair<double, double>, vector<Heap*>> param_to_heap_arr;
            for (const auto & param : params) {
                double eps = param.first;
                double delta = param.second;
                unsigned int chi = ceil(9.0 / (eps * eps) * log2(2.0 / (delta * eps)));
                vector<Heap *> heap_arr;
                for (int I = 0; I < ITERATIONS; I++) {
                    Heap* heap = new Heap(chi);
                    heap_arr.push_back(heap);
                }
                param_to_heap_arr.insert(pair<pair<double, double>, vector<Heap*>>(param,heap_arr));
            }

            unsigned int num_pkts = 0;
            auto pkt = pr.getNextIPPacket();
            while (pkt != nullptr) {
                for (const auto & param : params) {
                    for (auto & heap : param_to_heap_arr.at(param)) {
                        heap->Add(pkt->getReprString());
                    }
                }
                delete pkt;
                pkt = pr.getNextIPPacket();
                ++num_pkts;
            }

            for (const auto & param : params) {
                vector<map<string, double>> samples;
                for (auto & heap : param_to_heap_arr.at(param)) {
                    samples.push_back(heap->GetSample());
                }
                res.insert(pair<pair<double,double>, vector<map<string,double>>>(param, samples));
            }

            for (const auto & param : params) {
                for (auto & heap : param_to_heap_arr.at(param)) {
                    delete heap;
                }
            }

            return res;
        }

        int doNothing() {
            auto pkt = pr.getNextIPPacket();
            int num_pkts = 0;
            while (pkt != nullptr) {
                ++num_pkts;
                const auto& pkt_string = pkt->getReprString();
                delete pkt;
                pkt = pr.getNextIPPacket();
            }
            return num_pkts;
        }

        map<pair<double,double>, vector<map<string, double>>> frequencyEstimation(vector<pair<double, double>> params) {
            map<pair<double,double>, vector<map<string, double>>> res;
            std::random_device rd;

            map<pair<double, double>, vector<HyperLogLog>> param_to_hll_arr;
            for (const auto & param : params) {
                double eps = param.first;
                double delta = param.second;
                int counters = pow((3/eps),2);
                int bits = log2(counters);
                vector<HyperLogLog> hll_arr;
                for (int I = 0; I < ITERATIONS; I++) {
                    HyperLogLog hll(bits);
                    hll.set_seed(rd());
                    hll_arr.push_back(hll);
                }
                param_to_hll_arr.insert(pair<pair<double, double>, vector<HyperLogLog>>(param,hll_arr));
            }

            map<pair<double, double>, vector<Heap*>> param_to_heap_arr;
            for (const auto & param : params) {
                double eps = param.first;
                double delta = param.second;
                unsigned int chi = ceil(3.0 / (eps/2 * eps/2) * log2(2.0 / delta/2));
                vector<Heap*> heap_arr;
                for (int I = 0; I < ITERATIONS; I++) {
                    Heap* chiMax = new Heap(chi);
                    heap_arr.push_back(chiMax);
                }
                param_to_heap_arr.insert(pair<pair<double, double>, vector<Heap*>>(param,heap_arr));
            }

            int num_pkts = 0;
            std::clock_t start;
            start = std::clock();
            double duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
            cout << "pkts:" << num_pkts << " duration:" << duration << "[s]" << endl;
            auto pkt = pr.getNextIPPacket();
            while (pkt != nullptr) {
                ++num_pkts;
                if(num_pkts > stoi(getFrequencyLimit(dataset))) {
                    break;
                }
                const auto& pkt_string = pkt->getReprString();
                for (const auto & param : params) {
                    for (auto & hll : param_to_hll_arr.at(param)) {
                        hll.add(pkt_string.c_str(), pkt_string.size());
                    }
                    for (auto & heap : param_to_heap_arr.at(param)) {
                        heap->Add(pkt->getReprString());
                    }
                }
                delete pkt;
                if(num_pkts % 1000000 == 0) {
                    duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
                    cout << "pkts:" << num_pkts << " duration:" << duration << "[s]" << endl;
                }
                pkt = pr.getNextIPPacket();
            }
            for (const auto & param : params) {
                double eps = param.first;
                double delta = param.second;
                double chi = ceil(3.0 / (eps/2 * eps/2) * log2(2.0 / delta/2));
                cout << eps << " " << delta << " " << chi << endl;

                vector<double> Ps;
                vector<map<string, double>> samples;

                for (auto & hll : param_to_hll_arr.at(param)) {
                    double d = hll.estimate();
                    cout << "hll.estimate:" << d << endl;
                    Ps.push_back(chi/hll.estimate());
                }
                for (auto & heap : param_to_heap_arr.at(param)) {
                    const auto & s = heap->GetSample();
                    samples.push_back(s);
                }

                for (int k = 0; k < samples.size(); ++k) {
                    auto & sample = samples[k];
                    auto p = Ps[k];
                    cout << "p:" << p << endl;
                    for(const auto & s : sample) {
                        cout << "flow:" << s.first << endl;
                        cout << "sampled_frequency:" << s.second << endl;
                        double estimated_frequency = s.second/p;
                        sample[s.first] = estimated_frequency;
                        cout << "estimated_frequency:" << estimated_frequency << endl;
                    }
                    res[param] = samples;
                }
            }
            cout << "Finished preparing samples" << endl;

            for (const auto & param : params) {
                for (auto & heap : param_to_heap_arr.at(param)) {
                    delete heap;
                }
            }
            cout << "Finished deleting heaps" << endl;
            return res;
        }
    };
}

#endif //TDHH_ROUTER_HPP
