#include "PacketsReader.hpp"
#include <random>
#include <stdexcept>
#include <fstream>
#include "Hyperloglog.hpp"
#include "Router.hpp"
#include "Utils.hpp"
#include "MathUtils.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/algorithm/string.hpp>

using namespace TDHH;
using namespace std;
using namespace hll;

vector<double> volume_estimation(vector<int> counters, ofstream & result_stream, const string &dataset_file, DATASET dataset) {
    vector<double> estimations;
    Router router(dataset_file, dataset);
    vector<map<int, double> > result;
    const map<int, map<int,vector<double>>>& m = router.volumeEstimation(counters);
    for (const int & c : counters){
        const auto & pkts_to_estimation = m.at(c);
        for(const auto & item: pkts_to_estimation) {
            double num_pkts = item.first;
            const auto & pkt_estimations = item.second;
            for(const auto & est: pkt_estimations) {
                result_stream << c << "," << (int)num_pkts << "," << est << endl;
            }
        }
    }
    return estimations;
}

void frequency_estimation(vector<pair<double, double>> params, ofstream& result_stream, const string &dataset_file, const string &real_frequencies_file, DATASET dataset) {
    Router r(dataset_file, dataset);
    const auto & fe = r.frequencyEstimation(params);
    cout << "Got FE results" << endl;
    ifstream is(real_frequencies_file);
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
    cout << "Finished preparing morethans and ss" << endl;

    double number_of_flows = 0;
    while(is.good()) {
        double real_frequency;
        string flow;
        is >> real_frequency;
        is >> flow;
        ++number_of_flows;
        for (const auto & param : params) {
            std::clock_t start;
            start = std::clock();
            double eps = param.first;
            const vector<map<string, double>>& estimations_per_param = fe.at(param);
            for (int k = 0; k < estimations_per_param.size(); k++) {
                const map<string, double> & estimation = estimations_per_param[k];
                double estimated_frequency = 0;
                const auto & iter = estimation.find(flow);
                if (iter != estimation.end()) {
                    estimated_frequency = iter->second;
                }
                double diff = abs(real_frequency - estimated_frequency);
                if (diff > eps * S) {
                    morethans_per_param[param][k] += 1;
                }
                square_sum_per_param[param][k] += pow(diff,2);
            }
            double duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
            cout << "took:" << duration << "[s]" << endl;
        }
        if (int(number_of_flows) % 1000 == 0) {
            cout << "Finished reading frequencis of " << number_of_flows << endl;
        }
    }
    cout << "Before reporting results" << endl;

    for (const auto & param : params) {
        double eps = param.first;
        double delta = param.second;
        for (int k = 0; k < morethans_per_param[param].size(); k++) {
            result_stream << eps << "," << delta << "," << morethans_per_param[param][k] << "," << square_sum_per_param[param][k] << "," << number_of_flows << endl;
        }
    }
    cout << "Done." << endl;
}

void heavy_hitter(vector<pair<double, double>> params, double theta, const string &filename, string resDirectory,
                  const string &resfile, DATASET dataset) {
    Router router(filename, dataset);
    map<pair<double,double>, vector<string>> params_to_THH;
    map<pair<double,double>, vector<string>> params_to_miceFlows;
    map<pair<double,double>, vector<string>> params_to_otherFlows;

    for(const auto & param : params){
        double eps = param.first;
        double delta = param.second;
        ifstream ifsHH(resDirectory+to_string(theta)+string("-")+to_string(eps)+"THH.ser");
        ifstream ifsMice(resDirectory+to_string(theta)+string("-")+to_string(eps)+"mice.ser");
        ifstream ifsOther(resDirectory+to_string(theta)+string("-")+to_string(eps)+"other.ser");
        if (ifsHH.fail() || ifsMice.fail() || ifsOther.fail()){
            namespace fs = boost::filesystem;
            fs::path someDir(resDirectory);
            fs::directory_iterator end_iter;
            for(fs::directory_iterator dir_iter(someDir); dir_iter != end_iter ; ++dir_iter) {
                if (fs::is_regular_file(dir_iter->status())) {
                    string rs(dir_iter->path().c_str());
                    if(boost::starts_with(string(dir_iter->path().c_str()), resfile)) {
                        vector<string> THH;
                        vector<string> miceFlows;
                        vector<string> otherFlows;
                        ifstream is(rs);
                        double S;
                        is >> S;
                        while (is.good()) {
                            double real_frequency;
                            string flow;
                            is >> real_frequency;
                            is >> flow;
                            if (real_frequency >= S * theta) {
                                THH.push_back(flow);
                            } else if (real_frequency < S * (theta - eps)) {
                                miceFlows.push_back(flow);
                            } else {
                                otherFlows.push_back(flow);
                            }
                        }
                        params_to_THH.insert(pair<pair<double,double>, vector<string>> (param, THH));
                        params_to_miceFlows.insert(pair<pair<double,double>, vector<string>> (param, miceFlows));
                        params_to_otherFlows.insert(pair<pair<double,double>, vector<string>> (param, otherFlows));
                    }
                }
            }
            ofstream ofsHH(resDirectory+to_string(theta)+string("-")+to_string(eps)+"THH.ser");
            boost::archive::text_oarchive oa(ofsHH);
            oa << params_to_THH[param];
            ofstream ofsMice(resDirectory+to_string(theta)+string("-")+to_string(eps)+"mice.ser");
            boost::archive::text_oarchive ob(ofsMice);
            ob << params_to_miceFlows[param];
            ofstream ofsOther(resDirectory+to_string(theta)+string("-")+to_string(eps)+"other.ser");
            boost::archive::text_oarchive oc(ofsOther);
            oc << params_to_otherFlows[param];
        } else {
            boost::archive::text_iarchive ia(ifsHH);
            ia >> params_to_THH[param];
            boost::archive::text_iarchive ib(ifsMice);
            ib >> params_to_miceFlows[param];
            boost::archive::text_iarchive ic(ifsOther);
            ic >> params_to_otherFlows[param];
        }
    }
    const auto & samples = router.heavy_hitters(params);
    for(const auto & param : params) {
        vector<double> FPRs;
        vector<double> FNRs;
        double eps = param.first;
        double delta = param.second;
        double chi = ceil(9.0 / (eps * eps) * log2(2.0 / (delta * eps)));
        auto currTHH = params_to_THH.at(param);
        const auto &currMice = params_to_miceFlows.at(param);
        const auto &currOther = params_to_otherFlows.at(param);
        unsigned long universe_size = currTHH.size() + currMice.size() + currOther.size();
        for (const auto &sample : samples.at(param)) {
            vector<string> HH;
            for (const auto &item : sample) {
                string flow = item.first;
                double estimated_frequency = item.second;
                if (item.second >= (theta - eps / 2.0) * chi) {
                    HH.push_back(item.first);
                }
            }
            sort(HH.begin(), HH.end());
            sort(currTHH.begin(), currTHH.end());
            vector<string> difference;
            set_difference(HH.begin(), HH.end(), currTHH.begin(), currTHH.end(), back_inserter(difference));
            double FPR = double(difference.size()) / double(universe_size - currTHH.size());
            FPRs.push_back(FPR);
            vector<string> difference1;
            set_difference(currTHH.begin(), currTHH.end(), HH.begin(), HH.end(), back_inserter(difference1));
            double FNR = double(difference1.size()) / double(currTHH.size());
            FNRs.push_back(FNR);
        }
        cout << eps << "," << delta << "," << theta << ",";
        MEAN(0.0, FPRs, just_mean, false);
        MEAN(0.0, FNRs, just_mean, true);
    }
}

int main(int argc, char **argv) {
    if(argc < 3) {
        throw invalid_argument("Please provide test (VE, FE, HH) and dataset (CAIDA, UCLA, UCLA_FULL, UNIV1, UINV2, TEST).");
    }
    const TEST t = testEnum(argv[1]);
    const DATASET d = datasetEnum(argv[2]);
    const string results_path = "../results/";
    const string results_suffix = ".raw_res";
    const string datasets_path = "../datasets_files/";
    const string datasets_suffix = ".csv";

    const string test = testName(t);
    const string dataset = datasetName(d);
    string DATASET = dataset;
    boost::to_upper(DATASET);

    const string dataset_file = datasets_path + DATASET + "/" + dataset + datasets_suffix;
    const string result_file = results_path + test + "_" + dataset + results_suffix;
    const string real_frequencies_file = datasets_path + DATASET + "/" + dataset + "_flows_count-" + getFrequencyLimit(d) + datasets_suffix;

    bool addCSVHeader = false;
    ifstream check_result_stream;
    check_result_stream.open(result_file, fstream::in | fstream::out | fstream::app);
    if(!check_result_stream) {
        throw invalid_argument("Could not open " + result_file + " for reading.");
    }
    if(check_result_stream.peek() == EOF) {
        addCSVHeader = true;
    }
    check_result_stream.close();

    ofstream result_stream;
    result_stream.open(result_file, fstream::in | fstream::out | fstream::app);
    if(!result_stream) {
        throw invalid_argument("Could not open " + result_file + " for writing.");
    }

    switch(t) {
        case TEST::VE: {
            if(addCSVHeader) {
                result_stream << "counters,packets,estimation" << endl;
            }
            vector<int> counters = {128, 512, 1024};
            volume_estimation(counters, result_stream, dataset_file, d);
        }
        break;
        case TEST::FE: {
            if (addCSVHeader) {
                result_stream << "eps,delta,more_than,square_sum,number_of_flows" << endl;
            }
            vector<pair<double, double>> params;
            double epss[] = {0.1, 0.05, 0.01, 0.005};
            for(auto eps : epss) {
                for(int delta_pow = -2; delta_pow > -6; --delta_pow) {
                    double delta = pow(2, delta_pow);
                    params.emplace_back(eps, delta);
                }
            }
            frequency_estimation(params, result_stream, dataset_file, real_frequencies_file, d);
        }
        break;
        case TEST::HH: {
            if(addCSVHeader) {
                result_stream << "eps,delta,theta,FPR,FPR_upper_ci,FPR_lower_ci,FNR,FNR_upper_ci,FNR_lower_ci" << endl;
            }
            double theta = 0.01;
            vector<pair<double, double>> params;
            double epss[] = {0.005};
            for(auto eps : epss) {
                for(int delta_pow = -2; delta_pow > -6; --delta_pow) {
                    double delta = pow(2, delta_pow);
                    params.emplace_back(eps, delta);
                }
            }
            heavy_hitter(params, theta, dataset_file, "../datasets_files/" + DATASET + "/", result_file, d);
        }
        break;
    }

    result_stream.close();
    return 0;
}
