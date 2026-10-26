#ifndef BIT_STREAM_HPP
#define BIT_STREAM_HPP

#include <iostream>
#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief BitWriter facilitates bit-by-bit writing to an output stream.
 * It accumulates bits in an 8-bit buffer and flushes full bytes.
 */
class BitWriter {
public:
    explicit BitWriter(std::ostream& os);
    ~BitWriter();

    // Prevent copying
    BitWriter(const BitWriter&) = delete;
    BitWriter& operator=(const BitWriter&) = delete;

    /**
     * @brief Writes a single bit (0 or 1) to the stream.
     */
    void writeBit(int bit);

    /**
     * @brief Writes an ASCII string of '0' and '1' characters as raw bits.
     */
    void writeBits(const std::string& bitString);

    /**
     * @brief Writes a byte (8 bits) directly into the stream, accounting for current bit offset.
     */
    void writeByte(uint8_t byte);

    /**
     * @brief Flushes any remaining bits in the buffer, padding with 0s to complete the byte.
     */
    void flush();

    /**
     * @brief Returns total number of payload bits written.
     */
    uint64_t getBitsWritten() const { return totalBitsWritten; }

private:
    std::ostream& out;
    uint8_t buffer;
    int bitCount;
    uint64_t totalBitsWritten;
};

/**
 * @brief BitReader facilitates bit-by-bit reading from an input stream.
 */
class BitReader {
public:
    explicit BitReader(std::istream& is);

    // Prevent copying
    BitReader(const BitReader&) = delete;
    BitReader& operator=(const BitReader&) = delete;

    /**
     * @brief Reads a single bit from the stream.
     * @param bit Output parameter receiving 0 or 1.
     * @return true if a bit was successfully read, false if EOF is reached.
     */
    bool readBit(int& bit);

    /**
     * @brief Reads an 8-bit byte from the stream.
     * @param byte Output parameter receiving the byte.
     * @return true if successful, false on EOF.
     */
    bool readByte(uint8_t& byte);

private:
    std::istream& in;
    uint8_t buffer;
    int bitCount;
};

#endif // BIT_STREAM_HPP

