//
// Created by jalilm on 22/03/2018.
//

#include "PacketsReader.hpp"
#include <map>

namespace TDHH {

    using namespace std;
    using namespace TDHH;

    map<string, string> PacketsReader::getNextUCLAPacket(const CSVIterator& it) {
        map<string, string> res;
        int i = 0;
        while(i < (*it).size()){
            pair<string, string> p;
            switch(i){
                case 0:
                    p = pair<string, string>("id", std::to_string(id++));
                    break;
                case 1:
                    p = pair<string, string>("IP_SRC", (*it)[1]);
                    break;
                case 2:
                    p = pair<string, string>("IP_DST", (*it)[2]);
                    break;
                case 3:
                    p = pair<string, string>("PORT_SRC", (*it)[3]);
                    break;
                case 4:
                    p = pair<string, string>("PORT_DST", (*it)[4]);
                    break;
                case 5:
                    if ((*it)[5].compare("U") == 0) {
                        res.insert(pair<string, string>("length", std::to_string(stoi((*it)[6]) + 64))); // add 64 bytes of headers.
                        res.insert(pair<string, string>("proto", "UDP (17)"));
                    } else {
                        res.insert(pair<string, string>("length", std::to_string(stoi((*it)[5]) + 64))); // add 64 bytes of headers.
                        res.insert(pair<string, string>("proto", "TCP (6)"));
                    }
                    break;
                default:
                    return res;
            }
            res.insert(p);
            ++i;
        }
        return res;
    }

    map<string, string> PacketsReader::getNextUNIVPacket(const CSVIterator& it) {
        map<string, string> res;
        int i = 0;
        while(i < (*it).size()){
            pair<string, string> p;
            switch(i){
                case 0:
                    p = pair<string, string>("id", (*it)[0]);
                    break;
                case 3:
                    p = pair<string, string>("IP_SRC", (*it)[3]);
                    break;
                case 4:
                    p = pair<string, string>("IP_DST", (*it)[4]);
                    break;
                case 5:
                    p = pair<string, string>("PORT_SRC", (*it)[5]);
                    break;
                case 6:
                    p = pair<string, string>("PORT_DST", (*it)[6]);
                    break;
                case 1:
                    p = pair<string, string>("proto", (*it)[1]);
                    break;
                case 2:
                    p = pair<string, string>("length", (*it)[2]);
                    break;
                default:
                    return res;
            }
            res.insert(p);
            ++i;
        }
        return res;
    }

    map<string, string> PacketsReader::getNextCAIDAPacket(const CSVIterator& it) {
        map<string, string> res;
        for (int j = 0; j < (*it).size(); j++) {
            string field = (*it)[j];
            unsigned long pos = field.find(":");
            string key(field.substr(0, pos));
            string value(field.substr(pos + 1, field.length()));
            res.insert(pair<string, string>(key, value));
        }
        return res;
    }

    map<string, string> PacketsReader::getNextPacket(const CSVIterator& it) {
        if (dataset == CAIDA) {
            return getNextCAIDAPacket(it);
        } else if (dataset == UCLA) {
            return getNextUCLAPacket(it);
        } else if (dataset == UNIV) {
            return getNextUNIVPacket(it);
        } else {
            throw std::runtime_error("Not CAIDA nor UCLA dataset");
        }
    }

    IPPacket *PacketsReader::getNextIPPacket() {
        if (it == CSVIterator()) {
            return NULL;
        }
        const map<string, string> & m = getNextPacket(it++);
        return new IPPacket(m.find(string("IP_SRC"))->second, m.find(string("IP_DST"))->second,
                            stoi(m.find(string("id"))->second));
    }

    WeightedIPPacket *PacketsReader::getNextWeightedIPPacket() {
        if (it == CSVIterator()) {
            return NULL;
        }

        map<string, string> m = getNextPacket(it++);
        while(m.size() < 6) { // This packet does not have a weight, so we skip it.
            m = getNextPacket(it++);
        }

        return new WeightedIPPacket(m.find(string("IP_SRC"))->second, m.find(string("IP_DST"))->second,
                                    stoi(m.find(string("id"))->second), stoi(m.find(string("length"))->second));
    }

    TransportPacket *PacketsReader::getNextTransportPacket() {
        if (it == CSVIterator()) {
            return NULL;
        }
        const map<string, string> & m = getNextPacket(it++);
        auto proto = m.find(string("proto"))->second;
        auto is_tcp = proto.compare(string("TCP (6)")) == 0;
        auto is_udp = proto.compare(string("UDP (17)")) == 0;
        if (!is_tcp && !is_udp) {
            ++it;
            return getNextTransportPacket();
        }
        int port_src;
        int port_dst;
        try {
            port_src = stoi(m.find(string("PORT_SRC"))->second);
            port_dst = stoi(m.find(string("PORT_DST"))->second);

        } catch (...) {
            ++it;
            return getNextTransportPacket();
        }
        return new TransportPacket(m.find(string("IP_SRC"))->second, m.find(string("IP_DST"))->second, port_src,
                                   port_dst, m.find(string("proto"))->second, stoi(m.find(string("id"))->second));
    }
}