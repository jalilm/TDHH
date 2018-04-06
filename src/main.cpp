#include "PacketsReader.hpp"
#include <random>
#include "Hyperloglog.hpp"
#include "Router.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>

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
        cout << x.first << "," << msre << endl;
        msres.push_back(msre);
    }
    return msres;
}

vector<double> vol_est(double eps, double delta, const char* filename) {
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
vector<double> weighted_vol_est(double eps, double delta, const char* filename) {
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

vector<map<string,double> > dist_sample(double eps, double delta, const char* filename) {
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

vector<map<string,double> > weighted_dist_sample(double eps, double delta, const char* filename="../datasets_files/test.csv") {
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
        double S;
        is >> S;
        double flow_number = 1;
        double more_than = 0;
        while(is.good()) {
            double real_freq;
            string flow;
            is >> real_freq;
            is >> flow;
            double est_freq = sample[flow]/p;
            double diff = abs(real_freq - est_freq);
            if (diff > eps * S) {
                ++more_than;
            }
            cout << flow_number << "," << more_than/flow_number << endl;
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
        double B;
        is >> B;
        double flow_number = 1;
        double more_than = 0;
        while(is.good()) {
            double real_freq;
            string flow;
            is >> real_freq;
            is >> flow;
            double est_freq = sample[flow]/p;
            double diff = abs(real_freq - est_freq);
            if (diff > eps * B) {
                ++more_than;
            }
            cout << flow_number << "," << more_than/flow_number << endl;
            ++flow_number;
        }
    }
}

void heavy_hitter(double eps, double delta, double teta, const char* filename, string resDirectory, string resfile) {
    vector<map<string,double> > samples;
    Router router(filename);
    map<unsigned int, vector<string> > packets_to_THH;
    map<unsigned int, vector<string> > packets_to_miceFlows;
    map<unsigned int, vector<string> > packets_to_otherFlows;

    std::ifstream ifsHH(resDirectory+std::to_string(teta)+string("-")+std::to_string(eps)+"THH.ser");
    std::ifstream ifsMice(resDirectory+std::to_string(teta)+string("-")+std::to_string(eps)+"mice.ser");
    std::ifstream ifsOther(resDirectory+std::to_string(teta)+string("-")+std::to_string(eps)+"other.ser");
    if (ifsHH.fail() || ifsMice.fail() || ifsOther.fail()){
        namespace fs = boost::filesystem;
        fs::path someDir(resDirectory);
        fs::directory_iterator end_iter;
        for(fs::directory_iterator dir_iter(someDir); dir_iter != end_iter ; ++dir_iter) {
            if (fs::is_regular_file(dir_iter->status())) {
                string rs(dir_iter->path().c_str());
                if(boost::starts_with(string(dir_iter->path().filename().c_str()), resfile)) {
                    vector<string> THH;
                    vector<string> miceFlows;
                    vector<string> otherFlows;
                    std::ifstream is(rs);
                    double S;
                    is >> S;
                    while (is.good()) {
                        double real_freq;
                        string flow;
                        is >> real_freq;
                        is >> flow;
                        if (real_freq >= S * teta) {
                            THH.push_back(flow);
                        } else if (real_freq < S * (teta - eps)) {
                            miceFlows.push_back(flow);
                        } else {
                            otherFlows.push_back(flow);
                        }
                    }
                    packets_to_THH.insert(pair<unsigned int, vector<string> >(S, THH));
                    packets_to_miceFlows.insert(pair<unsigned int, vector<string> >(S, miceFlows));
                    packets_to_otherFlows.insert(pair<unsigned int, vector<string> >(S, otherFlows));
                }
            }
        }
        std::ofstream ofsHH(resDirectory+std::to_string(teta)+string("-")+std::to_string(eps)+"THH.ser");
        boost::archive::text_oarchive oa(ofsHH);
        oa << packets_to_THH;
        std::ofstream ofsMice(resDirectory+std::to_string(teta)+string("-")+std::to_string(eps)+"mice.ser");
        boost::archive::text_oarchive ob(ofsMice);
        ob << packets_to_miceFlows;
        std::ofstream ofsOther(resDirectory+std::to_string(teta)+string("-")+std::to_string(eps)+"other.ser");
        boost::archive::text_oarchive oc(ofsOther);
        oc << packets_to_otherFlows;
    } else {
        boost::archive::text_iarchive ia(ifsHH);
        ia >> packets_to_THH;
        boost::archive::text_iarchive ib(ifsMice);
        ib >> packets_to_miceFlows;
        boost::archive::text_iarchive ic(ifsOther);
        ic >> packets_to_otherFlows;
    }
    unsigned int chi = ceil(9.0 / (eps * eps) * log2(2.0 / (delta * eps)));
    for (int i = 0; i < ITERATIONS; ++i) {
        map<unsigned int, map<string, double> > samples = router.heavy_hitters(eps, delta);
        for(const auto & ms : samples) {
            vector<string> HH;
            const auto &s = ms.second;
            unsigned int num_pkts = ms.first;
            for (const auto &item : s) {
                if (item.second >= (teta - eps / 2.0) * chi) {
                    HH.push_back(item.first);
                }
            }
            auto currTHH = packets_to_THH.at(num_pkts);
            const auto & currMice = packets_to_miceFlows.at(num_pkts);
            const auto & currOther = packets_to_miceFlows.at(num_pkts);
            unsigned int universe_size = currTHH.size() + currMice.size() + currOther.size();
            std::sort(HH.begin(), HH.end());
            std::sort(currTHH.begin(), currTHH.end());
            std::vector<string> difference;
            std::set_difference(HH.begin(), HH.end(), currTHH.begin(), currTHH.end(), std::back_inserter(difference));
            double FPR = double(difference.size()) / double(universe_size - currTHH.size());
            std::vector<string> difference1;
            std::set_difference(currTHH.begin(), currTHH.end(), HH.begin(), HH.end(), std::back_inserter(difference1));
            double FNR = double(difference1.size()) / double(currTHH.size());
            cout << num_pkts << "," << FPR << "," << FNR << endl;
        }
        router.reset();
    }
}

void weighted_heavy_hitter(double eps, double delta, double teta, const char* filename, string resfile) {
    vector<map<string,double> > samples;
    Router router(filename);
    vector<string> THH;
    vector<string> miceFlows;
    std::ifstream is(resfile);
    double S;
    is >> S;
    while(is.good()) {
        double real_freq;
        string flow;
        is >> real_freq;
        is >> flow;
        if (real_freq >= S*teta) {
            THH.push_back(flow);
        } else if (real_freq < S*(teta-eps)) {
            miceFlows.push_back(flow);
        }
    }
    for (int i = 0; i < ITERATIONS; ++i) {
        vector<string> HH;
        QMax qmax = router.weighted_heavy_hitters(eps, delta);
        const auto &v = qmax.getSample();
        for(const auto &item : v) {
            if(item.second >= (teta - eps/2.0)*qmax.q){
                HH.push_back(item.first);
            }
        }
        //TODO: produce the needed results.
        router.reset();
    }
}

int main() {
    std::ofstream out("../results/HH.txt");
    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
    //vol_est(0.05,1,"../datasets_files/UCLA/lasr.cs.ucla.edu/ddos/traces/public/trace5/UCLA5.csv");
    //weighted_vol_est(0.05,1,"../datasets_files/UCLA/lasr.cs.ucla.edu/ddos/traces/public/trace5/weighted_UCLA5.csv");
    //dist_sample(0.05, 0.05);
    //weighted_dist_sample(0.5, 0.5);
    //freq_est(0.01, 0.1, "../datasets_files/UCLA/lasr.cs.ucla.edu/ddos/traces/public/trace5/UCLA5.csv", "../datasets_files/UCLA5_flows_count.csv");
    //weighted_freq_est(0.04, 0.1, "../datasets_files/UCLA/lasr.cs.ucla.edu/ddos/traces/public/trace5/weighted_UCLA5.csv", "../datasets_files/UCLA5_weighted_flows_count.csv");
    heavy_hitter(0.1, 0.05, 0.001,"../datasets_files/CAIDA16/caida.csv", "../datasets_files/CAIDA16/" ,"caida_flows_count-");
    //weighted_heavy_hitter(0.1, 0.1, 0.1,"../datasets_files/pkts.csv", "../datasets_files/pkts_weighted_flows_count.csv");
    std::cout.rdbuf(coutbuf); //reset to standard output again
    return 0;
}