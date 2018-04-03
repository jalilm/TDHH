//
// Created by jalilm on 22/03/2018.
//

#ifndef NEWTDHH_PKTS_READER_HPP
#define NEWTDHH_PKTS_READER_HPP

#include <fstream>
#include <string>
#include "IPPacket.hpp"
#include "CSVIterator.hpp"

namespace TDHH {

    using namespace TDHH;
    using namespace std;

    class PacketsReader {
    private:
        string filename;
        ifstream infile;
        CSVIterator it;
    public:
        PacketsReader(string filename) : filename(filename) {
            infile = ifstream(filename);
            it = CSVIterator(infile);
        }

        void reset() {
            infile = ifstream(filename);
            it = CSVIterator(infile);
        }

        IPPacket *getNextIPPacket();

        WeightedIPPacket *getNextWeightedIPPacket();

        TransportPacket *getNextTransportPacket();
    };
}

#endif //NEWTDHH_PKTS_READER_HPP
