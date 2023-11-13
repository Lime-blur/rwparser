#include <cstdlib>

#include "renderware.h"
using namespace std;

namespace rw {

    bool HeaderInfo::read(istream &rw) {
        uint32 buf[3];
        rw.read((char *) buf, 12);
        if (rw.eof()) return false;
        type = buf[0];
        length = buf[1];
        build = buf[2];
        if (build & 0xFFFF0000) {
            version = ((build >> 14) & 0x3FF00) | ((build >> 16) & 0x3F) | 0x30000;
        }
        else version = build << 8;
        return true;
    }

    void ChunkNotFound(CHUNK_TYPE chunk, uint32 address) {
        cerr << "chunk " << hex << chunk << " not found at 0x";
        cerr << hex << address << endl;
        exit(1);
    }

    int8 readInt8(istream &rw) {
        int8 tmp;
        rw.read(reinterpret_cast <char *> (&tmp), sizeof(int8));
        return tmp;
    }

    uint8 readUInt8(istream &rw) {
        uint8 tmp;
        rw.read(reinterpret_cast <char *> (&tmp), sizeof(uint8));
        return tmp;
    }

    int16 readInt16(istream &rw) {
        int16 tmp;
        rw.read(reinterpret_cast <char *> (&tmp), sizeof(int16));
        return tmp;
    }

    uint16 readUInt16(istream &rw) {
        uint16 tmp;
        rw.read(reinterpret_cast <char *> (&tmp), sizeof(uint16));
        return tmp;
    }

    int32 readInt32(istream &rw) {
        int32 tmp;
        rw.read(reinterpret_cast <char *> (&tmp), sizeof(int32));
        return tmp;
    }

    uint32 readUInt32(istream &rw) {
        uint32 tmp;
        rw.read(reinterpret_cast <char *> (&tmp), sizeof(uint32));
        return tmp;
    }

    float32 readFloat32(istream &rw) {
        float32 tmp;
        rw.read(reinterpret_cast <char *> (&tmp), sizeof(float32));
        return tmp;
    }
}
