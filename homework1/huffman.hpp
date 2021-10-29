#pragma once

#include <fstream>

// Declarations of functions and classes that implement Huffman coding.
void compress(std::ifstream&, std::ofstream&, bool const);
void decompress(std::ifstream&, std::ofstream&, bool const);
