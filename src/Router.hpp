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
            void reset() { pr = PacketsReader(filename);};
            HyperLogLog volumeEstimation(double eps, double delta) {
                int bits = 8;
                HyperLogLog hll(bits);
                auto pkt = pr.getNextIPPacket();
                while(pkt != NULL) {
                    auto pkt_string = pkt->getReprString();
                    hll.add(pkt_string.c_str(), pkt_string.size());
                    delete pkt;
                    pkt = pr.getNextIPPacket();
                }
                return hll;
            };

    };
};


#endif //NEWTDHH_ROUTER_HPP
