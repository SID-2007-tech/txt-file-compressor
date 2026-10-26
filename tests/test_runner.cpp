#include "HuffmanCompressor.hpp"
#include "FileUtils.hpp"

#include <iostream>
#include <fstream>
#include <cassert>
#include <cstring>
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

bool filesAreEqual(const std::string& p1, const std::string& p2) {
    std::ifstream f1(p1, std::ios::binary);
    std::ifstream f2(p2, std::ios::binary);

    if (!f1.is_open() || !f2.is_open()) return false;

    std::vector<char> b1(4096);
    std::vector<char> b2(4096);

    while (f1.good() && f2.good()) {
        f1.read(b1.data(), b1.size());
        f2.read(b2.data(), b2.size());
        if (f1.gcount() != f2.gcount()) return false;
        if (std::memcmp(b1.data(), b2.data(), f1.gcount()) != 0) return false;
    }

    return f1.eof() && f2.eof();
}

void testFile(const std::string& testName, const std::string& content) {
    std::cout << "[RUNNING] " << testName << "... ";

    std::string testDir = "tests/temp_test_dir";
    fs::create_directories(testDir);

    std::string originalPath = testDir + "/" + testName + ".txt";

    // Write original file
    {
        std::ofstream out(originalPath, std::ios::binary);
        out.write(content.data(), content.size());
    }

    HuffmanCompressor compressor;

    // 1. Compress
    auto compStats = compressor.compress(originalPath);

    // Verify compressed file exists in the same directory
    std::string expectedCompPath = originalPath + ".huf";
    assert(compStats.outputPath == expectedCompPath);
    assert(fs::exists(expectedCompPath));
    assert(fs::path(expectedCompPath).parent_path() == fs::path(originalPath).parent_path());

    // 2. Decompress to a separate test output file
    std::string decompPath = testDir + "/" + testName + "_restored.txt";
    auto decompStats = compressor.decompress(expectedCompPath, decompPath);

    assert(fs::exists(decompPath));
    assert(decompStats.originalSize == content.size());

    // 3. Compare byte-for-byte
    bool match = filesAreEqual(originalPath, decompPath);
    assert(match);

    std::cout << "PASSED (Original: " << compStats.originalSize
              << " B, Compressed: " << compStats.compressedSize << " B, Savings: "
              << compStats.spaceSavings << "%)\n";

    // Cleanup
    fs::remove(originalPath);
    fs::remove(expectedCompPath);
    fs::remove(decompPath);
}

void testDirectoryBatch() {
    std::cout << "[RUNNING] testDirectoryBatch... ";
    std::string testDir = "tests/temp_batch_dir";
    fs::create_directories(testDir);

    std::vector<std::string> fileNames = {"file1.txt", "file2.txt", "file3.txt"};
    for (const auto& name : fileNames) {
        std::ofstream out(testDir + "/" + name);
        out << "Sample test content for " << name << " with repeated data: AAAAAAAAAABBBBBBBCCCC";
    }

    auto txtFiles = FileUtils::findTxtFiles(testDir);
    assert(txtFiles.size() == 3);

    HuffmanCompressor compressor;
    for (const auto& f : txtFiles) {
        auto stats = compressor.compress(f);
        assert(fs::exists(stats.outputPath));
        assert(fs::path(stats.outputPath).parent_path() == fs::path(f).parent_path());
    }

    std::cout << "PASSED\n";
    fs::remove_all(testDir);
}

int main() {
    std::cout << "=======================================\n";
    std::cout << "     Huffman Compressor Unit Tests     \n";
    std::cout << "=======================================\n";

    try {
        // Test 1: Empty file
        testFile("empty_file", "");

        // Test 2: Single character repeated
        testFile("single_char", std::string(500, 'Z'));

        // Test 3: Standard English text
        std::string sampleText =
            "Operating Systems: Design and Implementation.\n"
            "Huffman coding is an entropy encoding algorithm used for lossless data compression.\n"
            "The algorithm's output can be viewed as a variable-length code table for encoding a source symbol.\n"
            "The algorithm derives this table from the estimated probability or frequency of occurrence for each possible value of the source symbol.";
        testFile("english_text", sampleText);

        // Test 4: All 256 byte values
        std::string allBytes;
        allBytes.reserve(256 * 10);
        for (int rep = 0; rep < 10; ++rep) {
            for (int i = 0; i < 256; ++i) {
                allBytes.push_back(static_cast<char>(i));
            }
        }
        testFile("all_256_bytes", allBytes);

        // Test 5: Directory batch processing
        testDirectoryBatch();

        // Cleanup temp directory
        fs::remove_all("tests/temp_test_dir");

        std::cout << "=======================================\n";
        std::cout << "   ALL TESTS COMPLETED SUCCESSFULLY!   \n";
        std::cout << "=======================================\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Test failed with exception: " << ex.what() << "\n";
        return 1;
    }
}
