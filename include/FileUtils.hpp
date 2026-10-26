#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

class FileUtils {
public:
    /**
     * @brief Checks if a path exists.
     */
    static bool exists(const std::string& path);

    /**
     * @brief Checks if a path is a regular file.
     */
    static bool isRegularFile(const std::string& path);

    /**
     * @brief Checks if a path is a directory.
     */
    static bool isDirectory(const std::string& path);

    /**
     * @brief Checks if a file path ends with a specific extension (case-insensitive).
     */
    static bool hasExtension(const std::string& path, const std::string& ext);

    /**
     * @brief Finds all .txt files in a directory (optionally recursive).
     */
    static std::vector<std::string> findTxtFiles(const std::string& dirPath, bool recursive = false);

    /**
     * @brief Derives the compressed output path in the same directory as the input file.
     * E.g. "/path/to/data.txt" -> "/path/to/data.txt.huf"
     */
    static std::string getCompressedPath(const std::string& inputPath);

    /**
     * @brief Derives the decompressed output path in the same directory.
     * E.g. "/path/to/data.txt.huf" -> "/path/to/data.txt"
     * If the target already exists, appends ".decompressed.txt" to avoid overwriting.
     */
    static std::string getDecompressedPath(const std::string& inputPath);

    /**
     * @brief Gets file size in bytes.
     */
    static uint64_t getFileSize(const std::string& path);

    /**
     * @brief Formats byte count into human-readable string (B, KB, MB).
     */
    static std::string formatBytes(uint64_t bytes);
};

#endif // FILE_UTILS_HPP

