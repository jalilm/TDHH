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

void vol_est(double eps, double delta=1){
    Router r("../files/pkts.csv");
    double real_vol = 30265204.0;
    for (int i=0; i < 10; i++) {
        HyperLogLog hll = r.volumeEstimation(eps, delta);
        double estimate = hll.estimate();
        cout << estimate <<","<< real_vol<<","<< 1.0 - estimate/real_vol << endl;
        r.reset();
    }
}

void dist_sample(double eps, double delta) {
    Router r("../files/pkts.csv");
    QMax qmax = r.sample(eps, delta);
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
    vol_est(0.05);
    return 0;
}