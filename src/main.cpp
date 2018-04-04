#include "PacketsReader.hpp"
#include <random>
#include "Hyperloglog.hpp"
#include "Router.hpp"

using namespace TDHH;
using namespace std;
using namespace hll;

const int ITERATIONS = 1;

template <typename A, typename B>
multimap<B, A> flip_map(map<A,B> & src) {
    multimap<B, A> dst;
    for (const auto & it: src) {
        dst.insert(pair<B, A>(it.second, it.first));
    }
    return dst;
}

template<typename t1, typename t2>
vector<double> MSRE(vector<map<t1, t2> >  & res) {
    vector<double> msres;
    auto const &vi = res[0];
    for (auto const &x : vi) {
        double square_sum = pow((x.second - x.first) / (x.first), 2);
        int i = 1;
        for (; i < res.size(); ++i) {
            const map<t1,t2>& y = res[i];
            const t2 & x1 = y.at(x.first);
            square_sum += pow((x1 - x.first) / (x.first), 2);
        }
        double msre = square_sum / double(i);
        //cout << x.first << "," << msre << endl;
        msres.push_back(msre);
    }
    return msres;
}

vector<double> vol_est(double eps, double delta = 1, const char* filename="../files/pkts.csv") {
    vector<double> estimations;
    Router router(filename);
    vector<map<int, double> > result;
    for (int i = 0; i < ITERATIONS; ++i) {
        const auto& m = router.volumeEstimation(eps, delta);
        result.push_back(m);
        estimations.push_back(m.rbegin()->second);
        router.reset();
    }
    MSRE(result);
    return estimations;
}

// TODO: the weighted_Add at hll, does not work as expected - only the o(w) variant works.
vector<double> weighted_vol_est(double eps, double delta = 1, const char* filename="../files/test.csv") {
    vector<double> estimations;
    Router router(filename);
    vector<map<unsigned long long int, double> > result;
    for (int i = 0; i < ITERATIONS; ++i) {
        const auto& m = router.weightedVolumeEstimation(eps, delta);
        result.push_back(m);
        estimations.push_back(m.rbegin()->second);
        router.reset();
    }
    MSRE(result);
    return estimations;
}

vector<map<string,double> > dist_sample(double eps, double delta, const char* filename="../files/test.csv") {
    vector<map<string,double> > samples;
    Router router(filename);
    for (int i = 0; i < ITERATIONS; ++i) {
        QMax qmax = router.sample(eps, delta);
        const auto &v = qmax.getSample();
        samples.push_back(v);
        router.reset();
    }
    return samples;
}

vector<map<string,double> > weighted_dist_sample(double eps, double delta, const char* filename="../files/test.csv") {
    vector<map<string,double> > samples;
    Router router(filename);
    for (int i = 0; i < ITERATIONS; ++i) {
        QMax qmax1 = router.weighted_sample(eps, delta);
        const auto& v = qmax1.getSample();
        samples.push_back(v);
        router.reset();
    }
    return samples;
}

void freq_est(double eps, double delta, const char * filename, string resfile) {
    const auto &estimations = vol_est(eps / 2, delta / 2, filename);
    const auto &samples = dist_sample(eps / 2, delta / 2, filename);
    for (int i = 0; i < ITERATIONS; ++i) {
        const auto &estimate = estimations.at(i);
        auto sample = samples.at(i);
        map<string, int> int_sample;
        double q = sample.size();
        double p = q / estimate;
//        for (const auto &s :sample) {
//            int_sample.insert(pair<string,int>(s.first, s.second/p));
//        }
//
//        const auto & reverseTest = flip_map(int_sample);
//        cout << "\nContents of flipped map in descending order:\n" << endl;
//        for (multimap<int, string>::const_reverse_iterator it = reverseTest.rbegin(); it != reverseTest.rend(); ++it)
//            cout << it->first << " " << it->second << endl;
//
//        cout << endl;
        std::ifstream is(resfile);
        double square_sum = 0;
        double flow_number = 1;
        while(is.good()) {
            double real_freq;
            string flow;
            is >> real_freq;
            is >> flow;
            double est_freq = sample[flow]/p;
            square_sum += pow((real_freq - est_freq) / real_freq, 2);
            cout << flow_number << "," << square_sum/flow_number << endl;
            ++flow_number;
        }
    }
}

void weighted_freq_est(double eps, double delta, const char * filename, string resfile) {
    const auto &estimations = weighted_vol_est(eps / 2, delta / 2, filename);
    const auto &samples = weighted_dist_sample(eps / 2, delta / 2, filename);
    for (int i = 0; i < ITERATIONS; ++i) {
        const auto &estimate = estimations.at(i);
        auto sample = samples.at(i);
        map<string, int> int_sample;
        double q = sample.size();
        double p = q / estimate;
//        for (const auto &s :sample) {
//            int_sample.insert(pair<string,int>(s.first, s.second/p));
//        }
//
//        const auto & reverseTest = flip_map(int_sample);
//        cout << "\nContents of flipped map in descending order:\n" << endl;
//        for (multimap<int, string>::const_reverse_iterator it = reverseTest.rbegin(); it != reverseTest.rend(); ++it)
//            cout << it->first << " " << it->second << endl;
//
//        cout << endl;
        std::ifstream is(resfile);
        double square_sum = 0;
        double flow_number = 1;
        while(is.good()) {
            double real_freq;
            string flow;
            is >> real_freq;
            is >> flow;
            double est_freq = sample[flow]/p;
            square_sum += pow((real_freq - est_freq) / real_freq, 2);
            cout << flow_number << "," << square_sum/flow_number << endl;
            ++flow_number;
        }
    }
}

void heavy_hitter(double eps, double delta, double teta, const char* filename, string resfile) {
    vector<map<string,double> > samples;
    Router router(filename);
    for (int i = 0; i < ITERATIONS; ++i) {
        QMax qmax = router.heavy_hitters(eps, delta);
        const auto &v = qmax.getSample();
        router.reset();
    }
}

void weighted_heavy_hitter(double eps, double delta, double teta, const char* filename, string resfile) {
    vector<map<string,double> > samples;
    Router router(filename);
    for (int i = 0; i < ITERATIONS; ++i) {
        QMax qmax = router.weighted_heavy_hitters(eps, delta);
        const auto &v = qmax.getSample();
        router.reset();
    }
}

int main() {
    std::ofstream out("../results/weighted_freq_est.txt");
    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
    //vol_est(0.05);
    //weighted_vol_est(0.05);
    //dist_sample(0.05, 0.05);
    //weighted_dist_sample(0.5, 0.5);
    //freq_est(0.1, 0.1, "../files/pkts.csv", "../files/pkts_flows_count.csv");
    weighted_freq_est(0.1, 0.1, "../files/pkts.csv", "../files/pkts_weighted_flows_count.csv");
    //heavy_hitter(0.1, 0.1, 0.1,"../files/pkts.csv", "../files/pkts_flows_count.csv");
    //weighted_heavy_hitter(0.1, 0.1, 0.1,"../files/pkts.csv", "../files/pkts_weighted_flows_count.csv");
    std::cout.rdbuf(coutbuf); //reset to standard output again
    return 0;
}