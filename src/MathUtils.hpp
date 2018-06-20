#ifndef TDHH_MATHUTILS_HPP
#define TDHH_MATHUTILS_HPP

using namespace std;

#include <cmath>
#include <vector>
#include <cstdio>

// Calculates the Square relative error of an observation.
template<typename t1>
t1 SRE(t1 real_result, t1 observation) {
    return pow((observation - real_result) / (real_result), 2);
}

// Just Calculates the mean.
template<typename t1>
t1 just_mean(t1 real_result, t1 observation) {
    return observation;
}

// Calculates the confidence interval of a vector of observations.
template<typename t1>
double CI(t1 mean, const vector<t1> &observations, double zvalue = 1.96) {
    double sum = 0;
    for (auto obs : observations) {
        sum += pow(obs - mean, 2);
    }
    double std = sqrt(sum / double(observations.size()));
    double sqr_size = sqrt(double(observations.size()));
    return std / sqr_size * zvalue;
}

// Calculates the mean and the confidence interval of a vector of observations.
template<typename t1>
void MEAN(t1 real_result, const vector<t1> &observations, t1 (*func)(t1, t1), bool printNewLine = true,
          double zvalue = 1.96) {
    vector<double> sums;
    auto const &vi = observations[0];
    double sum = 0;
    for (const auto &x : observations) {
        double observation_effect = func(real_result, x);
        sum += observation_effect;
        sums.push_back(observation_effect);
    }
    double mean = sum / double(observations.size());
    double ci = CI(mean, sums, zvalue);
    double upper_limit = mean + ci;
    double lower_limit = mean - ci;
    cout << real_result << "," << mean << "," << upper_limit << "," << lower_limit;
    if (printNewLine) {
        cout << endl;
    } else {
        cout << ",";
    }
}

#endif //TDHH_MATHUTILS_HPP
