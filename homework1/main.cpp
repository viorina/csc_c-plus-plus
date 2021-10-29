#include "huffman.hpp"

#include <iostream>

void print_error(std::string const& program_name, std::string const& message) {
    std::string usage = "\nUsage: " + program_name + " (-v) -c|-d <input filename> <output filename>";
    std::cout << message << usage << std::endl;
}

int main(int argc, char* argv[]) {
    if ((argc != 4) && (argc != 5)) {
        print_error(argv[0], "Wrong number of arguments!");
        return 0;
    }
    if ((argc == 5) && (std::string(argv[1]) != "-v")) {
        print_error(argv[0], "Invalid arguments!");
        return 0;
    }

    std::string mode = argv[argc - 3];
    if ((mode != "-c") && (mode != "-d")) {
        print_error(argv[0], "Invalid arguments!");
        return 0;
    }

    std::ifstream file_in(argv[argc - 2], std::ios::binary);
    std::ofstream file_out(argv[argc - 1], std::ios::binary);

    bool const print_stats = argc == 5;
    if (mode == "-c") {
        compress(file_in, file_out, print_stats);
    } 
    if (mode == "-d") {
        decompress(file_in, file_out, print_stats);
    }

    return 0;
}
