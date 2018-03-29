//
// Created by jalilm on 26/03/18.
//

#ifndef NEWTDHH_ROUTER_HPP
#define NEWTDHH_ROUTER_HPP

#include <string>
#include "PacketsReader.hpp"
#include "hyperloglog.hpp"
#include "QMax.hpp"
#include <map>
#include <boost/functional/hash.hpp>
#include <boost/math/special_functions/beta.hpp>

namespace TDHH {
    using namespace std;
    using namespace hll;

    class Router {
        protected:
            string filename;
            PacketsReader pr;
        public:
            Router(const char *filename) : filename(filename), pr(filename) {};
            void reset() { pr.reset();};
            map<int,double> volumeEstimation(double eps, double delta=0) {
                map<int, double> res;
                std::random_device rd;
                int bits = ceil(log2(pow(eps,-2)));
                HyperLogLog hll(bits);
                hll.set_seed(rd());
                auto pkt = pr.getNextIPPacket();
                int i = 0;
                while(pkt != NULL) {
                    ++i;
                    auto pkt_string = pkt->getReprString();
                    hll.add(pkt_string.c_str(), pkt_string.size());
                    delete pkt;
                    if (i % 1000000 == 0) {
                        res.insert(pair<int,double>(i, hll.estimate()));
                    }
                    pkt = pr.getNextIPPacket();
                }
                res.insert(pair<int,double>(i, hll.estimate()));
                return res;
            };
            QMax sample(double eps, double delta) {
                unsigned int chi = ceil(3.0/(eps*eps)*log2(2.0/delta));
                QMax chiMax = QMax(chi);
                auto pkt = pr.getNextIPPacket();
                while(pkt != NULL) {
                    chiMax.add(*pkt);
                    delete pkt;
                    pkt = pr.getNextIPPacket();
                }
                return chiMax;
            };

            template<typename URNG>
            double beta_sample(URNG& engine, double a, double b)
            {
                static std::uniform_real_distribution<double> unif(0,1);
                double p = unif(engine);
                return boost::math::ibeta_inv(a, b, p);
                // Use Boost policies if it's not fast enough
            }

            int hashCode(string s)
            {
                boost::hash<std::string> string_hash;
                return string_hash(s);
            }

            QMax weighted_sample(double eps, double delta) {
                unsigned int chi = ceil(3.0/(eps*eps)*log2(2.0/delta));
                QMax chiMax = QMax(chi);
                auto pkt = pr.getNextWeightedIPPacket();
                while(pkt != NULL) {
                    int w = pkt->weight;
                    int hc = hashCode(pkt->getReprString());
                    std::mt19937 prng(hc);
                    double p = beta_sample(prng, 1, w);
                    int i = 0;
                    bool added = true;
                    while ((i < w || i < chi) && added) {
                        added = chiMax.add_weighted(p, *pkt);
                        i++;
                        p *= beta_sample(prng, 1, w-i);
                    }
                    delete pkt;
                    pkt = pr.getNextWeightedIPPacket();
                }
                return chiMax;
            }
    };
};


#endif //NEWTDHH_ROUTER_HPP
