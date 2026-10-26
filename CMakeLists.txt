cmake_minimum_required(VERSION 3.10)
project(HuffmanCompression VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(COMMON_SOURCES
    src/BitStream.cpp
    src/HuffmanTree.cpp
    src/FileUtils.cpp
    src/HuffmanCompressor.cpp
)

add_executable(huffman src/main.cpp ${COMMON_SOURCES})

add_executable(test_runner tests/test_runner.cpp ${COMMON_SOURCES})

enable_testing()
add_test(NAME HuffmanUnitTests COMMAND test_runner)

