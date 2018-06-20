#include "PacketsReader.hpp"

namespace TDHH {

    using namespace std;
    using namespace TDHH;

    void PacketsReader::getNextUCLAPacket(const CSVIterator &it) {
        int i = 0;
        while (i < (*it).size()) {
            switch (i) {
                case 0:
                    id++;
                    break;
                case 1:
                    IP_SRC = (*it)[1];
                    break;
                case 2:
                    IP_DST = (*it)[2];
                    break;
                case 3:
                    PORT_SRC = (*it)[3];
                    break;
                case 4:
                    PORT_DST = (*it)[4];
                    break;
                case 5:
                    if ((*it)[5].compare("U") == 0) {
                        length = stoi((*it)[6]) + 64; // add 64 bytes of headers.
                        proto = "UDP (17)";
                    } else {
                        length = stoi((*it)[5]) + 64; // add 64 bytes of headers.
                        proto = "TCP (6)";
                    }
                    break;
            }
            ++i;
        }
    }

    void PacketsReader::getNextUNIVPacket(const CSVIterator &it) {
        id = stoi((*it)[0]);
        proto = (*it)[1];
        length = stoi((*it)[2]);
        IP_SRC = (*it)[3];
        IP_DST = (*it)[4];
        PORT_SRC = (*it)[5];
        PORT_DST = (*it)[6];
    }

    void PacketsReader::getNextCAIDAPacket(const CSVIterator &it) {
        unsigned long pos;
        string field = (*it)[0];
        pos = field.find(":");
        id = stoi(field.substr(pos + 1, field.length()));

        field = (*it)[1];
        pos = field.find(":");
        proto = field.substr(pos + 1, field.length());

        field = (*it)[2];
        pos = field.find(":");
        length = stoi(field.substr(pos + 1, field.length()));

        field = (*it)[3];
        pos = field.find(":");
        IP_SRC = field.substr(pos + 1, field.length());

        field = (*it)[4];
        pos = field.find(":");
        IP_DST = field.substr(pos + 1, field.length());

        field = (*it)[5];
        pos = field.find(":");
        PORT_DST = field.substr(pos + 1, field.length());

        field = (*it)[6];
        pos = field.find(":");
        PORT_SRC = field.substr(pos + 1, field.length());
    }

    void PacketsReader::getNextPacket(const CSVIterator &it) {
        switch (dataset) {
            case DATASET::CAIDA:
                return getNextCAIDAPacket(it);
            case DATASET::UCLA:
            case DATASET::UCLA_FULL:
                return getNextUCLAPacket(it);
            case DATASET::UNIV1:
            case DATASET::UNIV2:
                return getNextUNIVPacket(it);
        }
    }

    IPPacket *PacketsReader::getNextIPPacket() {
        if (it == CSVIterator()) {
            return NULL;
        }
        getNextPacket(it++);
        return new IPPacket(IP_SRC, IP_DST, id);
    }

    TransportPacket *PacketsReader::getNextTransportPacket() {
        if (it == CSVIterator()) {
            return NULL;
        }
        getNextPacket(it++);
        return new TransportPacket(IP_SRC, IP_DST, PORT_SRC, PORT_DST, proto, id);
    }
}