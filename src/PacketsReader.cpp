//
// Created by jalilm on 22/03/2018.
//

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


//        map<string, string> res;
//        int i = 0;
//        while(i < (*it).size()){
//            pair<string, string> p;
//            switch(i){
//                case 0:
//                    p = pair<string, string>("id", std::to_string(id++));
//                    break;
//                case 1:
//                    p = pair<string, string>("IP_SRC", (*it)[1]);
//                    break;
//                case 2:
//                    p = pair<string, string>("IP_DST", (*it)[2]);
//                    break;
//                case 3:
//                    p = pair<string, string>("PORT_SRC", (*it)[3]);
//                    break;
//                case 4:
//                    p = pair<string, string>("PORT_DST", (*it)[4]);
//                    break;
//                case 5:
//                    if ((*it)[5].compare("U") == 0) {
//                        res.insert(pair<string, string>("length", std::to_string(stoi((*it)[6]) + 64))); // add 64 bytes of headers.
//                        res.insert(pair<string, string>("proto", "UDP (17)"));
//                    } else {
//                        res.insert(pair<string, string>("length", std::to_string(stoi((*it)[5]) + 64))); // add 64 bytes of headers.
//                        res.insert(pair<string, string>("proto", "TCP (6)"));
//                    }
//                    break;
//                default:
//                    return res;
//            }
//            res.insert(p);
//            ++i;
//        }
//        return res;
    }

    void PacketsReader::getNextUNIVPacket(const CSVIterator &it) {
        id = stoi((*it)[0]);
        proto = (*it)[1];
        length = stoi((*it)[2]);
        IP_SRC = (*it)[3];
        IP_DST = (*it)[4];
        PORT_SRC = (*it)[5];
        PORT_DST = (*it)[6];
//        map<string, string> res;
//        int i = 0;
//        while(i < (*it).size()){
//            pair<string, string> p;
//            switch(i){
//                case 0:
//                    p = pair<string, string>("id", (*it)[0]);
//                    break;
//                case 3:
//                    p = pair<string, string>("IP_SRC", (*it)[3]);
//                    break;
//                case 4:
//                    p = pair<string, string>("IP_DST", (*it)[4]);
//                    break;
//                case 5:
//                    p = pair<string, string>("PORT_SRC", (*it)[5]);
//                    break;
//                case 6:
//                    p = pair<string, string>("PORT_DST", (*it)[6]);
//                    break;
//                case 1:
//                    p = pair<string, string>("proto", (*it)[1]);
//                    break;
//                case 2:
//                    p = pair<string, string>("length", (*it)[2]);
//                    break;
//                default:
//                    return res;
//            }
//            res.insert(p);
//            ++i;
//        }
//        return res;
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

//        map<string, string> res;
//        for (int j = 0; j < (*it).size(); j++) {
//            string field = (*it)[j];
//            unsigned long pos = field.find(":");
//            string key(field.substr(0, pos));
//            string value(field.substr(pos + 1, field.length()));
//            res.insert(pair<string, string>(key, value));
//        }
//        return res;
    }

    void PacketsReader::getNextCAIDA18Packet(const CSVIterator &it) {
        id = stoi((*it)[0]);
        proto = (*it)[1];
        length = stoi((*it)[2]);
        IP_SRC = (*it)[3];
        IP_DST = (*it)[4];
        PORT_SRC = (*it)[5];
        PORT_DST = (*it)[6];
//        map<string, string> res;
//        vector<string> field_names = {"id", "proto", "length", "IP_SRC", "IP_DST", "PORT_SRC", "PORT_DST"};
//        for (int i = 0; i < (*it).size(); ++i) {
//            pair<string, string> p(field_names.at(i), (*it)[i]);
//            res.insert(p);
//        }
//        return res;
    }

    void PacketsReader::getNextPacket(const CSVIterator &it) {
        switch (dataset) {
            case DATASET::CAIDA:
                return getNextCAIDAPacket(it);
            case DATASET::CAIDA18:
                return getNextCAIDA18Packet(it);
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
        //const map<string, string> & m = getNextPacket(it++);
        getNextPacket(it++);
//        return new IPPacket(m.find(string("IP_SRC"))->second, m.find(string("IP_DST"))->second,
//                            stoi(m.find(string("id"))->second));
        return new IPPacket(IP_SRC, IP_DST, id);
    }

//    WeightedIPPacket *PacketsReader::getNextWeightedIPPacket() {
//        if (it == CSVIterator()) {
//            return NULL;
//        }
//
//        map<string, string> m = getNextPacket(it++);
//        while(m.size() < 6) { // This packet does not have a weight, so we skip it.
//            m = getNextPacket(it++);
//        }
//
//        return new WeightedIPPacket(m.find(string("IP_SRC"))->second, m.find(string("IP_DST"))->second,
//                                    stoi(m.find(string("id"))->second), stoi(m.find(string("length"))->second));
//    }
//
//    TransportPacket *PacketsReader::getNextTransportPacket() {
//        if (it == CSVIterator()) {
//            return NULL;
//        }
//        const map<string, string> & m = getNextPacket(it++);
//        auto proto = m.find(string("proto"))->second;
//        auto is_tcp = proto.compare(string("TCP (6)")) == 0;
//        auto is_udp = proto.compare(string("UDP (17)")) == 0;
//        if (!is_tcp && !is_udp) {
//            ++it;
//            return getNextTransportPacket();
//        }
//        int port_src;
//        int port_dst;
//        try {
//            port_src = stoi(m.find(string("PORT_SRC"))->second);
//            port_dst = stoi(m.find(string("PORT_DST"))->second);
//
//        } catch (...) {
//            ++it;
//            return getNextTransportPacket();
//        }
//        return new TransportPacket(m.find(string("IP_SRC"))->second, m.find(string("IP_DST"))->second, port_src,
//                                   port_dst, m.find(string("proto"))->second, stoi(m.find(string("id"))->second));
//    }
}