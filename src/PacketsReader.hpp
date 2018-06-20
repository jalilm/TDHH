#ifndef TDHH_PACKETS_READER_HPP
#define TDHH_PACKETS_READER_HPP

#include <fstream>
#include <string>
#include "Packet.hpp"
#include "CSVIterator.hpp"
#include <map>
#include <utility>
#include "Utils.hpp"

namespace TDHH {

    using namespace TDHH;
    using namespace std;

    class PacketsReader {

    private:
        DATASET dataset;
        string filename;
        ifstream infile;
        CSVIterator it;
        int id;
        int length;
        string proto;
        string IP_SRC;
        string IP_DST;
        string PORT_SRC;
        string PORT_DST;

        void getNextUCLAPacket(const CSVIterator &it);

        void getNextCAIDAPacket(const CSVIterator &it);

        void getNextUNIVPacket(const CSVIterator &it);

        void getNextPacket(const CSVIterator &it);


    public:
        explicit PacketsReader(string filename, DATASET dataset) : filename(std::move(std::move(filename))),
                                                                   dataset(dataset), id(0), length(0), proto(""),
                                                                   IP_SRC(""), IP_DST(""), PORT_SRC(""), PORT_DST("") {
            infile = ifstream(this->filename);
            it = CSVIterator(infile);
        }

        void reset() {
            infile = ifstream(filename);
            it = CSVIterator(infile);
            id = 0;
        }

        IPPacket *getNextIPPacket();

        TransportPacket *getNextTransportPacket();
    };
}

#endif //TDHH_PACKETS_READER_HPP
