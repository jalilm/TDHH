//
// Created by jalilm on 22/03/2018.
//

#include "PacketsReader.hpp"
#include "CSVIterator.hpp"
#include <string>
#include <map>

namespace TDHH {

    using namespace std;
    using namespace TDHH;

    map<string, string> getNextPacket(CSVIterator it) {
        map<string,string> res;
        for (int j = 0; j < (*it).size(); j++) {
            string field = (*it)[j];
            unsigned long pos = field.find(":");
            string key(field.substr(0,pos));
            string value(field.substr(pos+1,field.length()));
            res.insert(pair<string,string>(key, value));
        }
        return res;
    };

    IPPacket* PacketsReader::getNextIPPacket() {
        if (it == CSVIterator()) {
            return NULL;
        }
        map<string,string> m = getNextPacket(it++);
        return new IPPacket(m.find(string("IP_SRC"))->second, m.find(string("IP_DST"))->second, stoi(m.find(string("id"))->second));
    }

    TransportPacket* PacketsReader::getNextTransportPacket() {
        if (it == CSVIterator()) {
            return NULL;
        }
        map<string,string> m = getNextPacket(it++);
        auto proto = m.find(string("proto"))->second;
        auto is_tcp  = proto.compare(string("TCP (6)"))==0;
        auto is_udp = proto.compare(string("UDP (17)"))==0;
        if (!is_tcp && !is_udp) {
            it++;
            return getNextTransportPacket();
        }
        int port_src;
        int port_dst;
        try {
            port_src = stoi(m.find(string("PORT_SRC"))->second);
            port_dst = stoi(m.find(string("PORT_DST"))->second);

        } catch (...) {
            it++;
            return getNextTransportPacket();
        }
        return new TransportPacket(m.find(string("IP_SRC"))->second, m.find(string("IP_DST"))->second, port_src, port_dst, m.find(string("proto"))->second, stoi(m.find(string("id"))->second));
    }
}