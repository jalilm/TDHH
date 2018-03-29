#include "PacketsReader.hpp"
#include <string>
#include <iostream>
#include <random>
#include "hyperloglog.hpp"
#include "Router.hpp"
#include "QMax.hpp"

using namespace TDHH;
using namespace std;
using namespace hll;

void vol_est(double eps, double delta=1){
    Router r("../files/pkts.csv");
    double real_vol = 30265204.0;
    for (int i=0; i < 10; i++) {
        auto m = r.volumeEstimation(eps, delta);
        for (auto const & x : m) {
            cout << x.first << "," << x.second << "," << 1.0 - x.second/x.first << endl;
        }
        r.reset();
    }
}

void dist_sample(double eps, double delta) {
    Router r("../files/pkts.csv");
    QMax qmax1 = r.sample(eps, delta);
    auto v = qmax1.getSample();
    cout << "QMAX 1" << endl;
    for (auto const & x : v) {
        cout << x.hash << "," << x.p << endl;
    }
    r.reset();
    QMax qmax2 = r.sample(eps, delta);
    auto v2 = qmax2.getSample();
    cout << "QMAX 2" << endl;
    for (auto const & x : v2) {
        cout << x.hash << "," << x.p << endl;
    }
    qmax1.merge(qmax2);
    auto vm = qmax1.getSample();
    cout << "Merged" << endl;
    for (auto const & x : vm) {
        cout << x.hash << "," << x.p << endl;
    }
}

void weighted_dist_sample(double eps, double delta) {
    Router r("../files/pkts.csv");
    QMax qmax1 = r.weighted_sample(eps, delta);
}

int main() {
    weighted_dist_sample(0.5, 0.5);
    return 0;
}