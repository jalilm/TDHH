//
// Created by jalilm on 28/03/18.
//

#ifndef TDHH_QMAX_HPP
#define TDHH_QMAX_HPP

#include <stdint.h>
#include "IPPacket.hpp"
#include <vector>
#include <algorithm>

namespace TDHH {
    using namespace std;
    class QMaxItem {
    public:
        double hash;
        IPPacket p;
        QMaxItem() : hash(0.0) {};

        QMaxItem(double x, IPPacket p) : hash(x), p(p) {}

        bool operator<(const QMaxItem &rhs) const {
            // returns true if rhs hash is smaller than this->hash
            if (hash > rhs.hash) {
                return true;
            }
            return false;
        }

        bool operator>(const QMaxItem &rhs) const {
            return rhs < *this;
        }

        bool operator<=(const QMaxItem &rhs) const {
            return !(rhs < *this);
        }

        bool operator>=(const QMaxItem &rhs) const {
            return !(*this < rhs);
        }
    };

    class QMax {
    private:
        struct reverseComp {
            bool operator()(const QMaxItem &i1, const QMaxItem &i2) {
                return i1 < i2;
            }
        };


        vector<QMaxItem> qMinHeap;
        std::mt19937 *engine;

        double hash(string s) {
            static std::uniform_real_distribution<double> unif(0, 1);
            return unif(*engine);
        }

    public:
        unsigned int q;
        QMax(unsigned int q) : q(q) {
            std::random_device rd;
            engine = new std::mt19937(rd());
        }

        ~QMax() {
            delete engine;
        }

        void add(IPPacket p) {
            QMaxItem *a = new QMaxItem(hash(p.getReprString()), p);
            if (qMinHeap.size() < q) {
                qMinHeap.push_back(*a);
                push_heap(qMinHeap.begin(), qMinHeap.end(), reverseComp());
            } else {
                QMaxItem min_item = qMinHeap.front();
                if (*a < min_item) {
                    pop_heap(qMinHeap.begin(), qMinHeap.end(), reverseComp());
                    qMinHeap.pop_back();
                    qMinHeap.push_back(*a);
                    push_heap(qMinHeap.begin(), qMinHeap.end(), reverseComp());
                }
            }
            delete a;
        }

        bool add_weighted(double hash, IPPacket p) {
            QMaxItem *a = new QMaxItem(hash, p);
            bool added = false;
            if (qMinHeap.size() < q) {
                qMinHeap.push_back(*a);
                push_heap(qMinHeap.begin(), qMinHeap.end(), reverseComp());
                added = true;
            } else {
                QMaxItem min_item = qMinHeap.front();
                if (*a < min_item) {
                    pop_heap(qMinHeap.begin(), qMinHeap.end(), reverseComp());
                    qMinHeap.pop_back();
                    qMinHeap.push_back(*a);
                    push_heap(qMinHeap.begin(), qMinHeap.end(), reverseComp());
                    added = true;
                }
            }
            delete a;
            return added;
        }

        map<string, double> getSample() const {
            map<string, double> sample;
            for (const auto& item : qMinHeap) {
                string flow = item.p.getFlowString();
                sample[flow]++;
            }
            return sample;
        }

        void merge(const QMax &rhs) {
            const auto& v2 = rhs.qMinHeap;
            qMinHeap.insert(qMinHeap.end(), v2.begin(), v2.end());
            sort_heap(qMinHeap.begin(), qMinHeap.end(), reverseComp());
            qMinHeap.resize(q);
            make_heap(qMinHeap.begin(), qMinHeap.end(), reverseComp());
        }
    };
}


#endif //TDHH_QMAX_HPP
