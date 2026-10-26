#include "FileUtils.hpp"
#include <iomanip>
#include <sstream>
#include <algorithm>

bool FileUtils::exists(const std::string& path) {
    std::error_code ec;
    return fs::exists(fs::u8path(path), ec);
}

bool FileUtils::isRegularFile(const std::string& path) {
    std::error_code ec;
    return fs::is_regular_file(fs::u8path(path), ec);
}

bool FileUtils::isDirectory(const std::string& path) {
    std::error_code ec;
    return fs::is_directory(fs::u8path(path), ec);
}

bool FileUtils::hasExtension(const std::string& path, const std::string& ext) {
    fs::path p = fs::u8path(path);
    std::string fileExt = p.extension().string();

    std::string targetExt = ext;
    if (!targetExt.empty() && targetExt[0] != '.') {
        targetExt = "." + targetExt;
    }

    auto toLower = [](std::string s) {
        for (char& c : s) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return s;
    };

    return toLower(fileExt) == toLower(targetExt);
}

std::vector<std::string> FileUtils::findTxtFiles(const std::string& dirPath, bool recursive) {
    std::vector<std::string> txtFiles;
    std::error_code ec;

    if (recursive) {
        for (const auto& entry : fs::recursive_directory_iterator(fs::u8path(dirPath), ec)) {
            if (entry.is_regular_file() && hasExtension(entry.path().string(), ".txt")) {
                txtFiles.push_back(entry.path().string());
            }
        }
    } else {
        for (const auto& entry : fs::directory_iterator(fs::u8path(dirPath), ec)) {
            if (entry.is_regular_file() && hasExtension(entry.path().string(), ".txt")) {
                txtFiles.push_back(entry.path().string());
            }
        }
    }

    std::sort(txtFiles.begin(), txtFiles.end());
    return txtFiles;
}

std::string FileUtils::getCompressedPath(const std::string& inputPath) {
    fs::path p = fs::u8path(inputPath);
    // E.g. /dir/notes.txt -> /dir/notes.txt.huf
    return (p.parent_path() / (p.filename().string() + ".huf")).string();
}

std::string FileUtils::getDecompressedPath(const std::string& inputPath) {
    fs::path p = fs::u8path(inputPath);
    std::string filename = p.filename().string();

    // Strip trailing .huf if present
    const std::string hufExt = ".huf";
    if (filename.length() > hufExt.length() &&
        filename.compare(filename.length() - hufExt.length(), hufExt.length(), hufExt) == 0) {
        filename = filename.substr(0, filename.length() - hufExt.length());
    } else {
        filename += ".decompressed.txt";
    }

    fs::path target = p.parent_path() / filename;

    // If destination already exists, avoid overwriting the original file
    if (fs::exists(target)) {
        fs::path stem = target.stem();
        fs::path ext = target.extension();
        target = p.parent_path() / (stem.string() + "_decompressed" + ext.string());
    }

    return target.string();
}

uint64_t FileUtils::getFileSize(const std::string& path) {
    std::error_code ec;
    auto size = fs::file_size(fs::u8path(path), ec);
    return ec ? 0 : size;
}

std::string FileUtils::formatBytes(uint64_t bytes) {
    std::ostringstream ss;
    if (bytes < 1024) {
        ss << bytes << " B";
    } else if (bytes < 1024 * 1024) {
        ss << std::fixed << std::setprecision(2) << (bytes / 1024.0) << " KB";
    } else {
        ss << std::fixed << std::setprecision(2) << (bytes / (1024.0 * 1024.0)) << " MB";
    }
    return ss.str();
}

