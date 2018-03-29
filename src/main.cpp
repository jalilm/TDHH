#include "PacketsReader.hpp"
#include <random>
#include "Hyperloglog.hpp"
#include "Router.hpp"

using namespace TDHH;
using namespace std;
using namespace hll;

template<typename t1, typename t2>
void MSRE(vector<map<t1, t2> > res) {
    auto const &vi = res[0];
    for (auto const &x : vi) {
        double square_sum = pow((x.second - x.first) / (x.first), 2);
        int i = 1;
        for (; i < res.size(); ++i) {
            const auto &y = res[i];
            square_sum += (y.at(x.first) - x.first) * (y.at(x.first) - x.first);
        }
        double msre = square_sum / double(i);
        cout << x.first << "," << msre << endl;
    }
}

void vol_est(double eps, double delta = 1) {
    Router r("../files/pkts.csv");
    double real_vol = 30265204.0;
    vector<map<int, double> > res;
    for (int i = 0; i < 10; i++) {
        auto m = r.volumeEstimation(eps, delta);
        res.push_back(m);
        r.reset();
    }
    MSRE(res);
}

// TODO: the weighted_Add at hll, does not work as expected
void w_vol_est(double eps, double delta = 1) {
    Router r("../files/pkts.csv");
    double real_w_vol = 14532166136.0;
    vector<map<unsigned long long int, double> > res;
    for (int i = 0; i < 1; i++) {
        auto m = r.weightedVolumeEstimation(eps, delta);
        res.push_back(m);
        for (auto const &x : m) {
            cout << x.first << "," << x.second << "," << 1.0 - x.second / x.first << endl;
        }
        r.reset();
    }
    MSRE(res);
}

void dist_sample(double eps, double delta) {
    Router r("../files/pkts.csv");
    QMax qmax1 = r.sample(eps, delta);
    auto v = qmax1.getSample();
    cout << "QMAX 1" << endl;
    for (auto const &x : v) {
        cout << x.hash << "," << x.p << endl;
    }
    r.reset();
    QMax qmax2 = r.sample(eps, delta);
    auto v2 = qmax2.getSample();
    cout << "QMAX 2" << endl;
    for (auto const &x : v2) {
        cout << x.hash << "," << x.p << endl;
    }
    qmax1.merge(qmax2);
    auto vm = qmax1.getSample();
    cout << "Merged" << endl;
    for (auto const &x : vm) {
        cout << x.hash << "," << x.p << endl;
    }
}

void weighted_dist_sample(double eps, double delta) {
    Router r("../files/test.csv");
    QMax qmax1 = r.weighted_sample(eps, delta);
    auto v = qmax1.getSample();
    cout << "QMAX 1" << endl;
    for (auto const &x : v) {
        cout << x.hash << "," << x.p << endl;
    }
}

int main() {
    vol_est(0.05);
    //w_vol_est(0.05);
    //weighted_dist_sample(0.5, 0.5);
    return 0;
}