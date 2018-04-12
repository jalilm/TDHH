#include "PacketsReader.hpp"
#include <random>
#include "Hyperloglog.hpp"
#include "Router.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/algorithm/string.hpp>

using namespace TDHH;
using namespace std;
using namespace hll;

const int ITERATIONS1 = 1;

// Calculates the Square realative error of an observation.
template <typename t1>
t1 SRE(t1 real_result, t1 observation) {
    return pow((observation - real_result) / (real_result), 2);
}

// Just Calculates the mean.
template <typename t1>
t1 just_mean(t1 real_result, t1 observation) {
    return observation;
}

// Calculates the confidence interval of a vector of observations.
template <typename t1>
double CI(t1 mean, const vector<t1> & observations, double zvalue=1.96) {
    double sum = 0;
    for (auto obs : observations) {
        sum += pow(obs-mean,2);
    }
    double std = sqrt(sum/double(observations.size()));
    double sqr_size = sqrt(double(observations.size()));
    return std/sqr_size * zvalue;
}

// Calculates the mean and the confidence interval of a vector of observations.
template<typename t1>
void MEAN(t1 real_result, const vector<t1> &observations, t1 (*func)(t1, t1), bool printNewLine = true, double zvalue = 1.96) {
    vector<double> sums;
    auto const &vi = observations[0];
    double sum=0;
    for (const auto &x : observations) {
        double observation_effect = func(real_result, x);
        sum += observation_effect;
        sums.push_back(observation_effect);
    }
    double mean = sum / double(observations.size());
    double ci = CI(mean, sums, zvalue);
    double upper_limit = mean + ci;
    double lower_limit =  mean - ci;
    cout << real_result << "," << mean << "," << upper_limit << "," << lower_limit;
    if(printNewLine) {
        cout << endl;
    } else {
        cout << ",";
    }
}

vector<double> vol_est(vector<int> counters, string filename, bool print=false) {
    vector<double> estimations;
    Router router(filename);
    vector<map<int, double> > result;
    const map<int, map<int,vector<double>>>& m = router.volumeEstimation(counters);
    for (const int & c : counters){
        const auto & pkts_to_estimation = m.at(c);
        for(const auto & item: pkts_to_estimation) {
            double num_pkts = item.first;
            const auto & estimations = item.second;
            if(print) {
                cout << c << ",";
                MEAN(num_pkts, estimations, SRE);
            }
        }
    }
    return estimations;
}

vector<map<string,double> > dist_sample(double eps, double delta, const char* filename) {
    vector<map<string,double> > samples;
    Router router(filename);
    for (int i = 0; i < ITERATIONS1; ++i) {
        QMax qmax = router.sample(eps, delta);
        const auto &v = qmax.getSample();
        samples.push_back(v);
        router.reset();
    }
    return samples;
}

void freq_est(vector<pair<double,double>> params, string filename, string resfile) {
    Router r(filename);
    const auto & fe = r.freq_est(params);

    std::ifstream is(resfile);
    double S;
    is >> S;

    map<pair<double,double>, vector<double>> morethans_per_param;
    map<pair<double,double>, vector<double>> square_sum_per_param;
    for (const auto & param : params) {
        const auto & estimations_per_param = fe.at(param);
        for (int k = 0; k < estimations_per_param.size(); k++) {
            morethans_per_param[param].push_back(0.0);
            square_sum_per_param[param].push_back(0.0);
        }
    }

    double number_of_flows = 0;
    while(is.good()) {
        double real_freq;
        string flow;
        is >> real_freq;
        is >> flow;
        ++number_of_flows;
        for (const auto & param : params) {
            double eps = param.first;
            vector<map<string, double>> estimations_per_param = fe.at(param);
            for (int k = 0; k < estimations_per_param.size(); k++) {
                const auto & estimation = estimations_per_param[k];
                double est_freq = 0;
                const auto & iter = estimation.find(flow);
                if (iter != estimation.end()) {
                    est_freq = iter->second;
                }
                double diff = abs(real_freq - est_freq);
                if (diff > eps * S) {
                    morethans_per_param[param][k] += 1;
                }
                square_sum_per_param[param][k] += pow(diff,2);
            }
        }
    }
    for (const auto & param : params) {
        double eps = param.first;
        double delta = param.second;
        cout << eps << "," << delta << ",";
        vector<double> WEPs;
        vector<double> rmses;
        for (int k = 0; k < morethans_per_param[param].size(); k++) {
            WEPs.push_back(morethans_per_param[param][k]/number_of_flows);
            MEAN(0.0, WEPs, just_mean, false);
            rmses.push_back(sqrt(square_sum_per_param[param][k]/number_of_flows));
            MEAN(0.0, rmses, just_mean, true);
        }
    }
}

void heavy_hitter(double eps, double delta, double theta, const char* filename, string resDirectory, string resfile) {
    vector<map<string,double> > samples;
    Router router(filename);
    map<unsigned int, vector<string> > packets_to_THH;
    map<unsigned int, vector<string> > packets_to_miceFlows;
    map<unsigned int, vector<string> > packets_to_otherFlows;

    std::ifstream ifsHH(resDirectory+std::to_string(theta)+string("-")+std::to_string(eps)+"THH.ser");
    std::ifstream ifsMice(resDirectory+std::to_string(theta)+string("-")+std::to_string(eps)+"mice.ser");
    std::ifstream ifsOther(resDirectory+std::to_string(theta)+string("-")+std::to_string(eps)+"other.ser");
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
                        if (real_freq >= S * theta) {
                            THH.push_back(flow);
                        } else if (real_freq < S * (theta - eps)) {
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
        std::ofstream ofsHH(resDirectory+std::to_string(theta)+string("-")+std::to_string(eps)+"THH.ser");
        boost::archive::text_oarchive oa(ofsHH);
        oa << packets_to_THH;
        std::ofstream ofsMice(resDirectory+std::to_string(theta)+string("-")+std::to_string(eps)+"mice.ser");
        boost::archive::text_oarchive ob(ofsMice);
        ob << packets_to_miceFlows;
        std::ofstream ofsOther(resDirectory+std::to_string(theta)+string("-")+std::to_string(eps)+"other.ser");
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
    for (int i = 0; i < ITERATIONS1; ++i) {
        map<unsigned int, map<string, double> > samples = router.heavy_hitters(eps, delta);
        for(const auto & ms : samples) {
            vector<string> HH;
            const auto &s = ms.second;
            unsigned int num_pkts = ms.first;
            for (const auto &item : s) {
                if (item.second >= (theta - eps / 2.0) * chi) {
                    HH.push_back(item.first);
                }
            }
            auto currTHH = packets_to_THH.at(num_pkts);
            const auto & currMice = packets_to_miceFlows.at(num_pkts);
            const auto & currOther = packets_to_otherFlows.at(num_pkts);
            unsigned int universe_size = currTHH.size() + currMice.size() + currOther.size();
            std::sort(HH.begin(), HH.end());
            std::sort(currTHH.begin(), currTHH.end());
            std::vector<string> difference;
            std::set_difference(HH.begin(), HH.end(), currTHH.begin(), currTHH.end(), std::back_inserter(difference));
            double FPR = double(difference.size()) / double(universe_size - currTHH.size());
            std::vector<string> difference1;
            std::set_difference(currTHH.begin(), currTHH.end(), HH.begin(), HH.end(), std::back_inserter(difference1));
            double FNR = double(difference1.size()) / double(currTHH.size());
            cout << num_pkts/1000000 << "," << FPR << "," << FNR << endl;
        }
        router.reset();
    }
}


int main() {
    string test = "fe_";
    string dataset = "caida";
    string DATASET = dataset;
    boost::to_upper(DATASET);

    std::streambuf *coutbuf = std::cout.rdbuf();
    std::ofstream out("../results/" + test + dataset + ".res");
    std::cout.rdbuf(out.rdbuf());
    stringstream ss ;
    ss <<  "../datasets_files/" << DATASET << "/" << dataset << ".csv";
    string filename;
    ss >> filename;

    stringstream ss1;
    ss1 <<  "../datasets_files/" << DATASET << "/" << dataset << "_flows_count-31000000.csv";
    string resfile;
    ss1 >> resfile;

    if (boost::starts_with(test, "ve")) {
        cout << "counters,pkts,msre,upper_ci,lower_ci" << endl;
        vector<int> counters = {128, 512, 1024};
        vol_est(counters, filename, true);
    } else if (boost::starts_with(test, "fe")) {
        cout << "eps,delta,dummy_wep,wep,wep_upper_ci,wep_lower_ci,dummy_rmse,rmse,rmse_upper_ci,rmse_lower_ci" << endl;
        vector<pair<double, double>> params;
        params.push_back(pair<double, double >(0.1,0.1));
        freq_est(params, filename, resfile);
    } else if (boost::starts_with(test, "hh")) {
    }
    std::cout.rdbuf(coutbuf);
    return 0;
}

//int main() {
//    double epss[] = {0.01, 0.005, 0.001};
//    double delta = 0.1;
//    double theta = 0.01;
//    vector<int> counters = {128, 512, 1024};
//    std::streambuf *coutbuf = std::cout.rdbuf();
//    std::ofstream out("../results/WVE_caida_O(W)");
//    std::cout.rdbuf(out.rdbuf());
//    weighted_vol_est(counters, "../datasets_files/CAIDA16/caida.csv");
    //cout << "eps,delta,ucla-wep,ucla-rmse,caida-wep,caida-rmse" << endl;
    //for(int delta_pow = -2; delta_pow > -11; --delta_pow) {
//    for(auto eps : epss) {
        //delta = pow(2, delta_pow);
//        delta = 0.05;
//        cout << eps << "," << delta << ",";// << theta << ",";
//        freq_est(eps,delta,"../datasets_files/UCLA/UCLA.csv","../datasets_files/UCLA/ucla_flows_count-8000000.csv");
//        freq_est(eps,delta,"../datasets_files/CAIDA16/caida.csv","../datasets_files/CAIDA16/caida_flows_count-31000000.csv");
//        heavy_hitter(eps, delta, theta, "../datasets_files/UCLA/UCLA.csv", "../datasets_files/UCLA/", "ucla_flows_count-8000000");
//        cout << endl;
//
//    }
//    std::cout.rdbuf(coutbuf); //reset to standard output again
//    return 0;
//}
//vol_est(0.05,1,"../datasets_files/UCLA/lasr.cs.ucla.edu/ddos/traces/public/trace5/UCLA5.csv");
//heavy_hitter(0.1, 0.05, 0.001,"../datasets_files/CAIDA16/caida.csv", "../datasets_files/CAIDA16/" ,"caida_flows_count-");