#ifndef TDHH_PACKET_HPP
#define TDHH_PACKET_HPP

#include <string>
#include <sstream>
#include <utility>

namespace TDHH {

    class Packet {
    public:
        Packet() = default;

        virtual ~Packet() = default;

        virtual std::string getReprString() const = 0;
    };

    class IPPacket : public Packet {
    protected:
        std::string src_ip;
        std::string dst_ip;
        int id{};
    public:
        IPPacket() = default;

        ~IPPacket() override = default;

        IPPacket(std::string src_ip, std::string dst_ip, int id) : src_ip(std::move(std::move(src_ip))),
                                                                   dst_ip(std::move(std::move(dst_ip))), id(id) {}

        std::string getReprString() const override {
            std::stringstream sstm;
            sstm << src_ip << "." << dst_ip << "." << id;
            return sstm.str();
        }

        inline friend std::ostream &operator<<(std::ostream &ostream, IPPacket const &v) {
            ostream << std::string("IP_SRC:") << v.src_ip << ",IP_DST:" << v.dst_ip << ",ID:" << v.id;
            return ostream;
        }
    };

    class TransportPacket : public IPPacket {
    protected:
        std::string src_port;
        std::string dst_port;
        std::string protocol;
    public:
        TransportPacket() = default;

        ~TransportPacket() override = default;

        TransportPacket(std::string src_ip, std::string dst_ip, std::string src_port, std::string dst_port,
                        std::string protocol, int id) :
                IPPacket(std::move(src_ip), std::move(dst_ip), id),
                src_port(std::move(std::move(src_port))),
                dst_port(std::move(std::move(dst_port))),
                protocol(std::move(std::move(protocol))) {}

        std::string getReprString() const override {
            std::stringstream sstm;
            sstm << src_ip << "." << dst_ip << "." << id << "." << src_port << "." << dst_port;
            return sstm.str();
        }

        inline friend std::ostream &operator<<(std::ostream &ostream, TransportPacket const &v) {
            ostream << v.protocol << ",";
            ostream << std::string("IP_SRC:") << v.src_ip << ",IP_DST:" << v.dst_ip << ",ID:" << v.id;
            ostream << std::string("SRC_PORT:") << v.src_port << ",PORT_DST:" << v.dst_port;
            return ostream;
        }
    };
}

#endif //TDHH_PACKET_HPP
