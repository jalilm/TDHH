#include "Heap.hpp"
#include <algorithm>

using std::pop_heap;
using std::push_heap;
using std::string;

struct minHeapComparator {
    bool operator()(const HeapItem &i1, const HeapItem &i2) {
        return i1 > i2;
    }
};

bool Heap::Add(const string &item) {
    bool added = false;
    HeapItem heap_item(item, hash(item));
    if (minHeap.size() < chi) {
        minHeap.push_back(heap_item);
        push_heap(minHeap.begin(), minHeap.end(), minHeapComparator());
        added = true;
    } else {
        HeapItem min_item = GetMinimalItem();
        if (heap_item > min_item) {
            pop_heap(minHeap.begin(), minHeap.end(), minHeapComparator());
            minHeap.pop_back();
            minHeap.push_back(heap_item);
            push_heap(minHeap.begin(), minHeap.end(), minHeapComparator());
            added = true;
        }
    }
    return added;
}

unsigned long Heap::Size() const {
    return minHeap.size();
}

HeapItem Heap::GetMinimalItem() const {
    return minHeap.front();
}

std::vector<HeapItem> Heap::GetItems() const {
    return minHeap;
}

std::map<string, double> Heap::GetSample() const {
    std::map<string, double> res;
    auto items = GetItems();
    for (const auto &item : items) {
        string flow = item.GetPair().first;
        auto it(res.find(flow));
        if (it != res.end()) {
            it->second++;
        } else {
            res[flow] = 1;
        }
    }
    return res;
};

void Heap::Merge(const Heap &rhs) {
    const auto &v2 = rhs.GetItems();
    minHeap.insert(minHeap.end(), v2.begin(), v2.end());
    std::sort(minHeap.begin(), minHeap.end(), minHeapComparator());
    if (minHeap.size() > chi) {
        minHeap.erase(minHeap.begin() + chi, minHeap.end());
    }
    std::make_heap(minHeap.begin(), minHeap.end(), minHeapComparator());
}
