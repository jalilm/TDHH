//
// Created by jalilm on 26/03/18.
//

#ifndef TDHH_ROUTER_HPP
#define TDHH_ROUTER_HPP

#include <string>
#include "PacketsReader.hpp"
#include "Hyperloglog.hpp"
#include "QMax.hpp"
#include <map>
#include <boost/functional/hash.hpp>
#include <boost/math/special_functions/beta.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace TDHH {
    using namespace std;
    using namespace hll;

    const int ITERATIONS = 10;

    class Router {

    private:
        template<typename URNG>
        double beta_sample(URNG &engine, double a, double b) {
            if (b == 0) {
                return 0;
            }
            static std::uniform_real_distribution<double> unif(0, 1);
            double p = unif(engine);
            return boost::math::ibeta_inv(a, b, p); // Use Boost policies if it's not fast enough
        }

        int hashCode(string s) {
            boost::hash<std::string> string_hash;
            return string_hash(s);
        }

    protected:
        string filename;
        PacketsReader pr;

    public:
        Router(string filename) :
        filename(filename),
        pr(filename, boost::starts_with(filename, "../datasets_files/UCLA")? PacketsReader::UCLA :
                     boost::starts_with(filename, "../datasets_files/CAIDA")? PacketsReader::CAIDA :
                     PacketsReader::UNIV)
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
                counter_to_hll_arr.insert(pair<int, vector<HyperLogLog>>(c,hll_arr));
            }
            auto pkt = pr.getNextIPPacket();
            int num_pkts = 0;
            while (pkt != NULL) {
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

        map<pair<int, unsigned int>, pair<unsigned long long int,double>> weightedVolumeEstimation(vector<int> counters) {
            map<pair<int, unsigned int>, pair<unsigned long long int,double>> res;
            std::random_device rd;
            map<int, HyperLogLog> hll_arr;
            for (int c : counters) {
                HyperLogLog hll(log2(c));
                hll.set_seed(rd());
                hll_arr.insert(pair<int, HyperLogLog>(c, hll));
            }
            auto pkt = pr.getNextWeightedIPPacket();
            unsigned long long int i = 0;
            unsigned int num_pkts = 0;
            while (pkt != NULL) {
                const auto& pkt_string = pkt->getReprString();
                int w = pkt->weight;
                i += w;
                ++num_pkts;
                for(auto item : hll_arr) {
                    item.second.add_weighted(pkt_string.c_str(), pkt_string.size(), w);
                }
                delete pkt;
                if (num_pkts % 1000000 == 0) {
                    for(const auto & item: hll_arr) {
                        pair<int, unsigned int> sp1(item.first, num_pkts);
                        pair<unsigned long long int, double> sp2(i, item.second.estimate());
                        res.insert(pair<pair<int, unsigned int>, pair<unsigned long long int, double>>(sp1,sp2));
                    }
                }
                pkt = pr.getNextWeightedIPPacket();
            }
            for(const auto & item: hll_arr) {
                pair<int, unsigned int> sp1(item.first, num_pkts);
                pair<unsigned long long int, double> sp2(i, item.second.estimate());
                res.insert(pair<pair<int, unsigned int>, pair<unsigned long long int, double>>(sp1,sp2));
            }
            return res;
        }

        QMax sample(double eps, double delta, bool formula_chi = true) {
            unsigned int chi;
            if (formula_chi) {
                 chi = ceil(3.0 / (eps * eps) * log2(2.0 / delta));
            } else {
                chi = ceil(9.0 / (eps * eps) * log2(2.0 / (delta * eps)));
            }
            QMax chiMax = QMax(chi);
            auto pkt = pr.getNextIPPacket();
            while (pkt != NULL) {
                chiMax.add(*pkt);
                delete pkt;
                pkt = pr.getNextIPPacket();
            }
            return chiMax;
        }

        QMax weighted_sample(double eps, double delta, bool formula_chi = true) {
            unsigned int chi;
            if (formula_chi) {
                chi = ceil(3.0 / (eps * eps) * log2(2.0 / delta));
            } else {
                chi = ceil(9.0 / (eps * eps) * log2(2.0 / (delta * eps)));
            }
            QMax chiMax = QMax(chi);
            auto pkt = pr.getNextWeightedIPPacket();
            while (pkt != NULL) {
                int w = pkt->weight;
                int hc = hashCode(pkt->getReprString());
                std::mt19937 prng(hc);
                double p = beta_sample(prng, 1, w);
                int i = 0;
                bool added = true;
                int min = w < chi ? w : chi;
                while ((i < min) && added) {
                    added = chiMax.add_weighted(p, *pkt);
                    i++;
                    p *= beta_sample(prng, 1, w - i);
                }
                delete pkt;
                pkt = pr.getNextWeightedIPPacket();
            }
            return chiMax;
        }

        map<pair<double,double>, vector<map<string, double>>> heavy_hitters(vector<pair<double,double>> params) {
            map<pair<double,double>, vector<map<string, double>>> res;

            map<pair<double, double>, vector<QMax>> param_to_qmax_arr;
            for (const auto & param : params) {
                double eps = param.first;
                double delta = param.second;
                unsigned int chi = ceil(9.0 / (eps * eps) * log2(2.0 / (delta * eps)));
                vector<QMax> qmax_arr;
                for (int I = 0; I < ITERATIONS; I++) {
                    QMax chiMax = QMax(chi);
                    qmax_arr.push_back(chiMax);
                }
                param_to_qmax_arr.insert(pair<pair<double, double>, vector<QMax>>(param,qmax_arr));
            }

            unsigned int num_pkts = 0;
            auto pkt = pr.getNextIPPacket();
            while (pkt != NULL) {
                for (const auto & param : params) {
                    for (auto & qmax : param_to_qmax_arr.at(param)) {
                        qmax.add(*pkt);
                    }
                }
                delete pkt;
                pkt = pr.getNextIPPacket();
                ++num_pkts;
            }

            for (const auto & param : params) {
                vector<map<string, double>> samples;
                for (auto & qmax : param_to_qmax_arr.at(param)) {
                    samples.push_back(qmax.getSample());
                }
                res.insert(pair<pair<double,double>, vector<map<string,double>>>(param, samples));
            }
            return res;
        }

        QMax weighted_heavy_hitters(double eps, double delta) {
            return weighted_sample(eps, delta, false);
        }

        map<pair<double,double>, vector<map<string, double>>> freq_est(vector<pair<double,double>> params) {
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

            map<pair<double, double>, vector<QMax>> param_to_qmax_arr;
            for (const auto & param : params) {
                double eps = param.first;
                double delta = param.second;
                unsigned int chi = ceil(3.0 / (eps/2 * eps/2) * log2(2.0 / delta/2));
                vector<QMax> qmax_arr;
                for (int I = 0; I < ITERATIONS; I++) {
                    QMax chiMax = QMax(chi);
                    qmax_arr.push_back(chiMax);
                }
                param_to_qmax_arr.insert(pair<pair<double, double>, vector<QMax>>(param,qmax_arr));
            }

            auto pkt = pr.getNextIPPacket();
            int num_pkts = 0;
            while (pkt != NULL) {
                ++num_pkts;
                const auto& pkt_string = pkt->getReprString();
                for (const auto & param : params) {
                    for (auto & hll : param_to_hll_arr.at(param)) {
                        hll.add(pkt_string.c_str(), pkt_string.size());
                    }
                    for (auto & qmax : param_to_qmax_arr.at(param)) {
                        qmax.add(*pkt);
                    }
                }
                delete pkt;
                pkt = pr.getNextIPPacket();
            }
            for (const auto & param : params) {
                double eps = param.first;
                double delta = param.second;
                double chi = ceil(3.0 / (eps/2 * eps/2) * log2(2.0 / delta/2));

                vector<double> Ps;
                vector<map<string, double>> samples;

                for (auto & hll : param_to_hll_arr.at(param)) {
                    Ps.push_back(chi/hll.estimate());
                }
                for (auto & qmax : param_to_qmax_arr.at(param)) {
                    samples.push_back(qmax.getSample());
                }

                for (int k = 0; k < samples.size(); ++k) {
                    auto & sample = samples[k];
                    auto p = Ps[k];
                    for(const auto & s : sample) {
                        double est_freq = s.second/p;
                        sample[s.first] = est_freq;
                    }
                    res[param] = samples;
                }
            }
            return res;
        }
    };
}

#endif //TDHH_ROUTER_HPP
