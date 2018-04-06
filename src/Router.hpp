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
        Router(const char *filename) :
        filename(filename),
        pr(filename, boost::starts_with(filename, "../datasets_files/UCLA")?
                        PacketsReader::UCLA : PacketsReader::CAIDA)
        {}

        void reset() {
            pr.reset();
        }

        map<int, double> volumeEstimation(double eps, double delta = 0) {
            map<int, double> res;
            std::random_device rd;
            int bits = ceil(log2(pow(eps, -2)));
            HyperLogLog hll(bits);
            hll.set_seed(rd());
            auto pkt = pr.getNextIPPacket();
            int i = 0;
            while (pkt != NULL) {
                ++i;
                const auto& pkt_string = pkt->getReprString();
                hll.add(pkt_string.c_str(), pkt_string.size());
                delete pkt;
                if (i % 1000000 == 0) {
                    res.insert(pair<int, double>(i, hll.estimate()));
                }
                pkt = pr.getNextIPPacket();
            }
            res.insert(pair<int, double>(i, hll.estimate()));
            return res;
        }

        map<unsigned long long int, double> weightedVolumeEstimation(double eps, double delta = 0) {
            map<unsigned long long int, double> res;
            std::random_device rd;
            int bits = ceil(log2(pow(eps, -2)*5+33));
            HyperLogLog hll(bits);
            hll.set_seed(rd());
            auto pkt = pr.getNextWeightedIPPacket();
            unsigned long long int i = 0;
            int num_pkts = 0;
            while (pkt != NULL) {
                const auto& pkt_string = pkt->getReprString();
                int w = pkt->weight;
                i += w;
                ++num_pkts;
                hll.add_weighted(pkt_string.c_str(), pkt_string.size(), w);
                delete pkt;
                if (num_pkts % 1000000 == 0) {
                    res.insert(pair<unsigned long long int, double>(i, hll.estimate()));
                }
                pkt = pr.getNextWeightedIPPacket();
            }
            res.insert(pair<unsigned long long int, double>(i, hll.estimate()));
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

        map<unsigned int, map<string, double> > heavy_hitters(double eps, double delta) {
            map<unsigned int, map<string, double> > res;
            unsigned int chi = ceil(9.0 / (eps * eps) * log2(2.0 / (delta * eps)));
            unsigned int num_pkts = 0;
            QMax chiMax = QMax(chi);
            auto pkt = pr.getNextIPPacket();
            while (pkt != NULL) {
                chiMax.add(*pkt);
                delete pkt;
                pkt = pr.getNextIPPacket();
                ++num_pkts;
                if((num_pkts % 1000000) == 0) {
                    res.insert(pair<unsigned int, map<string, double> >(num_pkts, chiMax.getSample()));
                }
            }
            res.insert(pair<unsigned int, map<string, double> >(num_pkts, chiMax.getSample()));
            return res;
        }

        QMax weighted_heavy_hitters(double eps, double delta) {
            return weighted_sample(eps, delta, false);
        }
    };
}

#endif //TDHH_ROUTER_HPP
