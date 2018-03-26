#include "PacketsReader.hpp"
#include <string>
#include <iostream>
#include <random>
#include <boost/math/special_functions/beta.hpp>
#include "hyperloglog.hpp"
#include "Router.hpp"

using namespace TDHH;
using namespace std;
using namespace hll;



template<typename URNG>
double beta_sample(URNG& engine, double a, double b)
{
    static std::uniform_real_distribution<double> unif(0,1);
    double p = unif(engine);
    return boost::math::ibeta_inv(a, b, p);
    // Use Boost policies if it's not fast enough
}

int main() {
//    std::random_device rd;
//    std::mt19937 gen(rd());
//    double sum = 0;
//    int i = 0;
//    for(;i<100000;i++) {
//        sum += beta_sample(gen,1,1);
//    }
//    cout << sum/i << endl;
//    PacketsReader pr1 = PacketsReader(string("../files/pkts.csv"));
//    unsigned long i1 = 0;
//    auto pkt1 = pr1.getNextTransportPacket();
//    while (pkt1 != NULL) {
//        delete pkt1;
//        i1++;
//        pkt1 = pr1.getNextTransportPacket();f
//    }
//    cout << i1 << endl;
//    cout << pr1.bad_proto << endl;
//    cout << pr1.no_ports << endl;
    Router r("../files/pkts.csv");
    HyperLogLog hll = r.volumeEstimation(1,1);
    cout << hll.estimate() << endl;
    return 0;
}