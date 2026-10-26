#include "BitStream.hpp"

BitWriter::BitWriter(std::ostream& os)
    : out(os), buffer(0), bitCount(0), totalBitsWritten(0) {}

BitWriter::~BitWriter() {
    flush();
}

void BitWriter::writeBit(int bit) {
    buffer = (buffer << 1) | (bit & 1);
    bitCount++;
    totalBitsWritten++;
    if (bitCount == 8) {
        out.put(static_cast<char>(buffer));
        buffer = 0;
        bitCount = 0;
    }
}

void BitWriter::writeBits(const std::string& bitString) {
    for (char c : bitString) {
        writeBit(c == '1' ? 1 : 0);
    }
}

void BitWriter::writeByte(uint8_t byte) {
    for (int i = 7; i >= 0; --i) {
        writeBit((byte >> i) & 1);
    }
}

void BitWriter::flush() {
    if (bitCount > 0) {
        // Pad the remainder of the byte with 0 bits
        buffer <<= (8 - bitCount);
        out.put(static_cast<char>(buffer));
        buffer = 0;
        bitCount = 0;
    }
    out.flush();
}

BitReader::BitReader(std::istream& is)
    : in(is), buffer(0), bitCount(0) {}

bool BitReader::readBit(int& bit) {
    if (bitCount == 0) {
        int byteRead = in.get();
        if (byteRead == EOF) {
            return false;
        }
        buffer = static_cast<uint8_t>(byteRead);
        bitCount = 8;
    }

    bit = (buffer >> (bitCount - 1)) & 1;
    bitCount--;
    return true;
}

bool BitReader::readByte(uint8_t& byte) {
    byte = 0;
    for (int i = 0; i < 8; ++i) {
        int bit = 0;
        if (!readBit(bit)) {
            return false;
        }
        byte = (byte << 1) | bit;
    }
    return true;
}

