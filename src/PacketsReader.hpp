//
// Created by jalilm on 22/03/2018.
//

#ifndef TDHH_PKTS_READER_HPP
#define TDHH_PKTS_READER_HPP

#include <fstream>
#include <string>
#include "IPPacket.hpp"
#include "CSVIterator.hpp"
#include <map>


namespace TDHH {

    using namespace TDHH;
    using namespace std;

    class PacketsReader {

    public:
        enum Dataset {CAIDA, UCLA};

    private:
        Dataset dataset;
        string filename;
        ifstream infile;
        CSVIterator it;
        unsigned long long int id;

        map<string, string> getNextUCLAPacket(const CSVIterator& it);
        map<string, string> getNextCAIDAPacket(const CSVIterator& it);
        map<string, string> getNextPacket(const CSVIterator& it);

    public:
        PacketsReader(string filename, Dataset dataset=CAIDA) : filename(filename), dataset(dataset), id(0) {
            infile = ifstream(filename);
            it = CSVIterator(infile);
        }

        void reset() {
            infile = ifstream(filename);
            it = CSVIterator(infile);
            id = 0;
        }

        IPPacket *getNextIPPacket();

        WeightedIPPacket *getNextWeightedIPPacket();

        TransportPacket *getNextTransportPacket();
    };
}

#endif //TDHH_PKTS_READER_HPP
