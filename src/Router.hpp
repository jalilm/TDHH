//
// Created by jalilm on 26/03/18.
//

#ifndef NEWTDHH_ROUTER_HPP
#define NEWTDHH_ROUTER_HPP

#include <string>
#include "PacketsReader.hpp"
#include "hyperloglog.hpp"

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
            HyperLogLog volumeEstimation(double eps, double delta=0) {
                std::random_device rd;
                int bits = ceil(log2(pow(eps,-2)));
                HyperLogLog hll(bits);
                hll.set_seed(rd());
                auto pkt = pr.getNextIPPacket();
                while(pkt != NULL) {
                    auto pkt_string = pkt->getReprString();
                    hll.add(pkt_string.c_str(), pkt_string.size());
                    delete pkt;
                    pkt = pr.getNextIPPacket();
                }
                return hll;
            };
            QMax sample(double eps, double delta) {
                unsigned int chi = ceil(3.0/(eps*eps)*log2(2.0/delta));
                QMax chiMax = QMax(chi);
                auto pkt = pr.getNextIPPacket();
                while(pkt != NULL) {
                    auto pkt_string = pkt->getReprString();
                    chiMax.add(pkt_string.c_str(), pkt_string.size());
                    delete pkt;
                    pkt = pr.getNextIPPacket();
                }
                return chiMax;
            }

    };
};


#endif //NEWTDHH_ROUTER_HPP
