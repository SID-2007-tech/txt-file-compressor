#include "HuffmanCompressor.hpp"
#include "FileUtils.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

void printUsage(const char* progName) {
    std::cout << "========================================================\n";
    std::cout << "           Huffman File Compressor (C++17)             \n";
    std::cout << "========================================================\n";
    std::cout << "Usage:\n";
    std::cout << "  " << progName << " <path_to_file_or_directory>\n";
    std::cout << "  " << progName << " -c <path_to_file_or_directory>   # Explicitly compress\n";
    std::cout << "  " << progName << " -d <path_to_huf_file>            # Decompress\n";
    std::cout << "  " << progName << " -r <directory_path>              # Compress .txt files recursively\n";
    std::cout << "  " << progName << " -h | --help                      # Show this help\n\n";
    std::cout << "Features:\n";
    std::cout << "  - If given a path to a .txt file, it compresses it and\n";
    std::cout << "    places <filename>.huf in the same directory.\n";
    std::cout << "  - If given a path to a directory, it compresses all .txt\n";
    std::cout << "    files found in that directory into their respective directories.\n";
    std::cout << "  - If run without arguments, it will prompt you for a path.\n";
    std::cout << "========================================================\n";
}

void printStats(const CompressionStats& stats, bool isCompression) {
    std::cout << "\n--------------------------------------------------------\n";
    if (isCompression) {
        std::cout << " [SUCCESS] File Compressed Successfully!\n";
        std::cout << "--------------------------------------------------------\n";
        std::cout << " Source File:     " << stats.inputPath << "\n";
        std::cout << " Compressed File: " << stats.outputPath << "\n";
        std::cout << " Distinct Chars:  " << stats.distinctSymbols << "\n";
        std::cout << " Original Size:   " << FileUtils::formatBytes(stats.originalSize)
                  << " (" << stats.originalSize << " bytes)\n";
        std::cout << " Compressed Size: " << FileUtils::formatBytes(stats.compressedSize)
                  << " (" << stats.compressedSize << " bytes)\n";
        std::cout << " Compression Ratio: " << std::fixed << std::setprecision(2) << stats.compressionRatio << ":1\n";
        std::cout << " Space Savings:     " << std::fixed << std::setprecision(2) << stats.spaceSavings << "%\n";
    } else {
        std::cout << " [SUCCESS] File Decompressed Successfully!\n";
        std::cout << "--------------------------------------------------------\n";
        std::cout << " Compressed File:   " << stats.inputPath << "\n";
        std::cout << " Decompressed File: " << stats.outputPath << "\n";
        std::cout << " Restored Size:     " << FileUtils::formatBytes(stats.originalSize)
                  << " (" << stats.originalSize << " bytes)\n";
    }
    std::cout << " Elapsed Time:    " << std::fixed << std::setprecision(2) << stats.durationMs << " ms\n";
    std::cout << "--------------------------------------------------------\n";
}

int processPath(const std::string& rawPath, bool decompressMode, bool recursive) {
    // Strip surrounding quotes if user entered them in prompt
    std::string path = rawPath;
    if (path.size() >= 2 && ((path.front() == '"' && path.back() == '"') ||
                             (path.front() == '\'' && path.back() == '\''))) {
        path = path.substr(1, path.size() - 2);
    }

    if (!FileUtils::exists(path)) {
        std::cerr << "\n[Error] Path does not exist: " << path << "\n";
        return 1;
    }

    HuffmanCompressor compressor;

    // Check if it's a directory
    if (FileUtils::isDirectory(path)) {
        if (decompressMode) {
            std::cerr << "\n[Error] Directory decompression is not supported directly. Please specify a .huf file.\n";
            return 1;
        }

        std::cout << "\nScanning directory: " << path << " for .txt files...\n";
        auto txtFiles = FileUtils::findTxtFiles(path, recursive);

        if (txtFiles.empty()) {
            std::cout << "No .txt files found in directory: " << path << "\n";
            return 0;
        }

        std::cout << "Found " << txtFiles.size() << " .txt file(s). Starting compression...\n";
        int successCount = 0;
        uint64_t totalOriginal = 0;
        uint64_t totalCompressed = 0;

        for (const auto& file : txtFiles) {
            std::cout << "\n>> Compressing: " << file << " ...";
            try {
                auto stats = compressor.compress(file);
                printStats(stats, true);
                totalOriginal += stats.originalSize;
                totalCompressed += stats.compressedSize;
                successCount++;
            } catch (const std::exception& ex) {
                std::cerr << "\n[Error compressing " << file << "]: " << ex.what() << "\n";
            }
        }

        std::cout << "\n========================================================\n";
        std::cout << " Batch Compression Completed: " << successCount << "/" << txtFiles.size() << " files processed.\n";
        if (totalOriginal > 0) {
            double totalSavings = (1.0 - static_cast<double>(totalCompressed) / totalOriginal) * 100.0;
            std::cout << " Total Original Size:   " << FileUtils::formatBytes(totalOriginal) << "\n";
            std::cout << " Total Compressed Size: " << FileUtils::formatBytes(totalCompressed) << "\n";
            std::cout << " Overall Space Savings: " << std::fixed << std::setprecision(2) << totalSavings << "%\n";
        }
        std::cout << "========================================================\n";
        return 0;
    }

    // It's a single file
    if (FileUtils::isRegularFile(path)) {
        // Auto-detect decompression if file ends in .huf and mode not explicitly set to compress
        if (FileUtils::hasExtension(path, ".huf")) {
            decompressMode = true;
        }

        try {
            if (decompressMode) {
                std::cout << "\nDecompressing file: " << path << " ...\n";
                auto stats = compressor.decompress(path);
                printStats(stats, false);
            } else {
                std::cout << "\nCompressing file: " << path << " ...\n";
                auto stats = compressor.compress(path);
                printStats(stats, true);
            }
            return 0;
        } catch (const std::exception& ex) {
            std::cerr << "\n[Error]: " << ex.what() << "\n";
            return 1;
        }
    }

    std::cerr << "\n[Error] Invalid file type: " << path << "\n";
    return 1;
}

int main(int argc, char* argv[]) {
    bool decompressMode = false;
    bool recursive = false;
    std::string targetPath;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-c" || arg == "--compress") {
            decompressMode = false;
        } else if (arg == "-d" || arg == "--decompress") {
            decompressMode = true;
        } else if (arg == "-r" || arg == "--recursive") {
            recursive = true;
        } else if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        } else {
            targetPath = arg;
        }
    }

    // Interactive prompt if no path was provided on CLI
    if (targetPath.empty()) {
        printUsage(argv[0]);
        std::cout << "Enter path to a .txt file or directory containing .txt files: ";
        std::getline(std::cin, targetPath);

        // Trim whitespace
        while (!targetPath.empty() && (targetPath.front() == ' ' || targetPath.front() == '\t')) {
            targetPath.erase(targetPath.begin());
        }
        while (!targetPath.empty() && (targetPath.back() == ' ' || targetPath.back() == '\t' || targetPath.back() == '\r')) {
            targetPath.pop_back();
        }

        if (targetPath.empty()) {
            std::cerr << "No path provided. Exiting.\n";
            return 1;
        }
    }

    return processPath(targetPath, decompressMode, recursive);
}

