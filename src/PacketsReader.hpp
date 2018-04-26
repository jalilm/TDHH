//
// Created by jalilm on 22/03/2018.
//

#ifndef TDHH_PACKETS_READER_HPP
#define TDHH_PACKETS_READER_HPP

#include <fstream>
#include <string>
#include "Packet.hpp"
#include "CSVIterator.hpp"
#include <map>
#include <utility>

namespace TDHH {

    using namespace TDHH;
    using namespace std;

    class PacketsReader {

    public:
        enum Dataset {CAIDA, UCLA, UNIV};

    private:
        Dataset dataset;
        string filename;
        ifstream infile;
        CSVIterator it;
        unsigned long long int id;

        map<string, string> getNextUCLAPacket(const CSVIterator& it);
        map<string, string> getNextCAIDAPacket(const CSVIterator& it);
        map<string, string> getNextUNIVPacket(const CSVIterator& it);
        map<string, string> getNextPacket(const CSVIterator& it);


    public:
        explicit PacketsReader(string filename, Dataset dataset=CAIDA) : filename(std::move(std::move(filename))), dataset(dataset), id(0) {
            infile = ifstream(this->filename);
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

#endif //TDHH_PACKETS_READER_HPP
