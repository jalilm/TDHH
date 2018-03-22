//
// Created by jalilm on 22/03/2018.
//

#ifndef NEWTDHH_PACKET_HPP
#define NEWTDHH_PACKET_HPP

namespace TDHH {

#include <string>

    using namespace std;

    class IPPacket {
    protected:
        string src_ip;
        string dst_ip;
    public:
        IPPacket(string src_ip, string dst_ip) : src_ip(src_ip), dst_ip(dst_ip) {}

        inline std::ostream &operator<<(std::ostream &Str, IPPacket const &v) {
            Str << string("IP_SRC:") << v.src_ip << ",IP_DST:" << v.dst_ip;
            // print something from v to str, e.g: Str << v.getX();
            return Str;
        }
    };

    class TransportPacket : IPPacket {
    private:
        int src_port;
        int dst_port;
        string protocol;
    public:
        TransportPacket(string src_ip, string dst_ip, int src_port, int dst_port, string protocol) : IPPacket(src_ip,
                                                                                                              dst_ip),
                                                                                                     src_port(src_port),
                                                                                                     dst_port(dst_port),
                                                                                                     protocol(
                                                                                                             protocol) {}

        inline std::ostream &operator<<(std::ostream &Str, TransportPacket const &v) {
            Str << v.protocol << ",";
            Str << string("IP_SRC:") << v.src_ip << ",IP_DST:" << v.dst_ip << ",";
            Str << string("SRC_PORT:") << v.src_port << ",PORT_DST:" << v.dst_port;
            return Str;
        }
    };
}

#endif //NEWTDHH_PACKET_HPP
