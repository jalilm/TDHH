//
// Created by jalilm on 22/03/2018.
//

#ifndef NEWTDHH_PKTS_READER_HPP
#define NEWTDHH_PKTS_READER_HPP

namespace TDHH {

#include <string>
    #include <fstream>
    #include "IPPacket.hpp"

    using namespace TDHH;
    using namespace std;

    class PacketsReader {
    private:
        string filename;
        ifstream infile;
    public:
        PacketsReader(string filename) : filename(filename), infile(filename) {}

        IPPacket getNextIPPacket();

        TransportPacket getNextTransportPacket();
    };
}

#endif //NEWTDHH_PKTS_READER_HPP
