#ifndef TDHH_HEAP_HPP
#define TDHH_HEAP_HPP

#include <vector>
#include <map>
#include <random>
#include <string>
#include <cstdio>

using std::vector;
using std::mt19937;
using std::string;
using std::ostream;

class HeapItem {
private:
    double item_hash;
    std::string item;
public:
    HeapItem(std::string item, double item_hash) : item(std::move(item)), item_hash(item_hash) {}

    virtual ~HeapItem() = default;

    bool operator<(const HeapItem &rhs) const {
        return item_hash < rhs.item_hash;
    }

    bool operator>(const HeapItem &rhs) const {
        return rhs < *this;
    }

    bool operator<=(const HeapItem &rhs) const {
        return !(rhs < *this);
    }

    bool operator>=(const HeapItem &rhs) const {
        return !(*this < rhs);
    }

    bool operator==(const HeapItem &rhs) const {
        return item == rhs.item && item_hash == rhs.item_hash;
    }

    std::pair<std::string, double> GetPair() const {
        return std::pair<std::string, double>(item, item_hash);
    };

    friend std::ostream &operator<<(std::ostream &os, const HeapItem &i) {
        os << i.item << std::__cxx11::string(" ... ") << std::__cxx11::to_string(i.item_hash);
        return os;
    }
};

class Heap {
protected:
    unsigned int chi;
    std::mt19937 *engine;
    std::vector<HeapItem> minHeap;

    virtual double hash(const string &item) {
        static std::uniform_real_distribution<double> uniform(0, 1);
        return uniform(*engine);
    }

public:
    explicit Heap(unsigned int chi, bool useRandomSeed = true, unsigned int seed = 2711) : chi(chi) {
        if (useRandomSeed) {
            std::random_device rd;
            engine = new std::mt19937(rd());
        } else {
            engine = new std::mt19937(seed);
        }
    }

    virtual ~Heap() {
        delete engine;
    }

    virtual unsigned long size() const;

    virtual bool Add(string item);

    virtual HeapItem GetMinimalItem() const;

    virtual std::vector<HeapItem> GetItems() const;

    virtual std::map<string, double> GetSample() const;

    virtual void Merge(const Heap &rhs);
};

#endif //TDHH_HEAP_HPP
