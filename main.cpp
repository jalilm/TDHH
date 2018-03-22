#include <iostream>
#include "PacketsReader.hpp"

using namespace TDHH;
using namespace std;

int main() {
    PacketsReader p = PacketsReader(string("test.csv"));
    p.getNextIPPacket();
    return 0;
}