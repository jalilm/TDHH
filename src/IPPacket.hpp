//
// Created by jalilm on 22/03/2018.
//

#ifndef TDHH_PACKET_HPP
#define TDHH_PACKET_HPP

#include <string>
#include <sstream>

namespace TDHH {

    using namespace std;

    class IPPacket {
    protected:
        string src_ip;
        string dst_ip;
        int id;
    public:
        IPPacket() {};

        IPPacket(string src_ip, string dst_ip, int id) : src_ip(src_ip), dst_ip(dst_ip), id(id) {}

        inline friend std::ostream &operator<<(std::ostream &Str, IPPacket const &v) {
            Str << string("IP_SRC:") << v.src_ip << ",IP_DST:" << v.dst_ip << ",ID:" << v.id;
            return Str;
        }

        virtual string getReprString() const {
            stringstream sstm;
            sstm << src_ip << "." << dst_ip << "." << id;
            return sstm.str();
        }

        virtual string getFlowString() const {
            stringstream sstm;
            sstm << src_ip << ":" << dst_ip;
            return sstm.str();
        }
    };

    class WeightedIPPacket : public IPPacket {
    public:
        int weight;

        WeightedIPPacket() {};

        WeightedIPPacket(string src_ip, string dst_ip, int id, int length) : IPPacket(src_ip, dst_ip, id),
                                                                             weight(length) {};

        inline friend std::ostream &operator<<(std::ostream &Str, WeightedIPPacket const &v) {
            Str << string("IP_SRC:") << v.src_ip << ",IP_DST:" << v.dst_ip << ",ID:" << v.id << ",WEIGHT:" << v.weight;
            return Str;
        }
    };



    class TransportPacket : IPPacket {
    protected:
        int src_port;
        int dst_port;
        string protocol;
    public:
        TransportPacket(string src_ip, string dst_ip, int src_port, int dst_port, string protocol, int id) :
                IPPacket(src_ip,dst_ip, id),
                src_port(src_port),
                dst_port(dst_port),
                protocol(protocol) {}

        string getReprString() const{
            stringstream sstm;
            sstm << src_ip << "." << dst_ip << "." << id << "." << src_port << "." << dst_port;
            return sstm.str();
        }

        string getFlowString() const {
            stringstream sstm;
            sstm << src_ip << ":" << dst_ip << ":" << src_port << ":" << dst_port;
            return sstm.str();
        }

        inline friend std::ostream &operator<<(std::ostream &Str, TransportPacket const &v) {
            Str << v.protocol << ",";
            Str << string("IP_SRC:") << v.src_ip << ",IP_DST:" << v.dst_ip << ",ID:" << v.id;
            Str << string("SRC_PORT:") << v.src_port << ",PORT_DST:" << v.dst_port;
            return Str;
        }
    };
}

#endif //TDHH_PACKET_HPP
