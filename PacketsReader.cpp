//
// Created by jalilm on 22/03/2018.
//

#include "PacketsReader.hpp"
#include "CSVIterator.hpp"

namespace TDHH {

    using namespace std;
    using namespace TDHH;

    IPPacket PacketsReader::getNextIPPacket() {

        for (CSVIterator loop(infile); loop != CSVIterator(); ++loop) {
            cout << "4th Element(" << (*loop)[3] << ")\n";
        }
        return IPPacket("", "");
    }

    TransportPacket PacketsReader::getNextTransportPacket() {
        return TransportPacket("", "", 0, 0, "");
    }
}