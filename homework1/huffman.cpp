#include "huffman.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <queue>
#include <vector>

// Definition of functions and classes that implement Huffman coding.

namespace {
    uint8_t const N_BITS = 8;
    uint8_t const NODE_CHAR = 0;
    uint16_t const N_CHARS = 256;

    struct node {
        uint16_t c;
        uint32_t freq;
        node* left;
        node* right;
    };

    void encode(std::ifstream& input, std::string const* codes, std::ofstream& output) {
        uint8_t pos = N_BITS - 1;
        unsigned char buffer = 0;

        uint32_t output_size = 0;

        unsigned char c;
        while (input.read(reinterpret_cast<char*>(&c), sizeof(char))) {
            std::string str = codes[c];
            for (unsigned char bit : str) {
                buffer |= (bit - '0') << pos;
                if (pos-- == 0) {
                    output.write(reinterpret_cast<const char*>(&buffer), sizeof(char));
                    ++output_size;
                    pos = N_BITS - 1;
                    buffer = 0;
                }
            }
        }

        if (pos != N_BITS - 1) {
            output.write(reinterpret_cast<const char*>(&buffer), sizeof(char));
            ++output_size;
        }
        std::cout << output_size << std::endl;
    }

    void decode(uint32_t n_symbols, std::ifstream& input, node* tree, std::ofstream& output) {
        node* n = tree;

        uint32_t input_size = 0;
        uint32_t output_size = 0;

        unsigned char buffer;
        while (input.read(reinterpret_cast<char*>(&buffer), 1)) {
            ++input_size;

            for (int i = N_BITS - 1; i >= 0 && n_symbols; --i) {
                int bit = ((int(buffer) & (1 << i)) != 0);
                if (n->left) {
                    n = (bit ? n->right : n->left);
                }

                if (!n->left) {
                    uint16_t c = n->c;
                    output << char(c);
                    ++output_size;
                    --n_symbols;
                    n = tree;
                }
            }
        }
        std::cout << input_size << "\n" << output_size << std::endl;
    }

    std::string* get_codes(node* root) {
        std::queue< std::pair<node*, std::string> > queue;
        queue.push(std::make_pair(root, root->left ? "" : "0"));

        std::string* table = new std::string [N_CHARS] ();

        while (!queue.empty()) {
            std::pair<node*, std::string> pair = queue.front();
            node* left = pair.first->left;
            std::string code = pair.second;
            queue.pop();

            if (left) {
                queue.push(std::make_pair(left, code + "0"));
                queue.push(std::make_pair(pair.first->right, code + "1"));
            } else {
                table[pair.first->c] += code;
            }
        }
        return table;
    }

    node* build_tree(uint32_t const* freqs) {
        auto cmp = [](node* lhs, node* rhs) {
            return (lhs->freq == rhs->freq ? lhs->c > rhs->c : lhs->freq > rhs->freq); 
        };
        std::priority_queue<node*, std::vector<node*>, decltype(cmp)> queue(cmp);

        for (uint16_t i = 0; i < N_CHARS; ++i) {
            if (freqs[i] > 0) {
                node* n = new node { i, freqs[i], nullptr, nullptr };
                queue.push(n);
            }
        }

        while (queue.size() != 1) {
            node* left = queue.top();
            queue.pop();
            node* right = queue.top();
            queue.pop();
            node* root = new node { NODE_CHAR, left->freq + right->freq, left, right };
            queue.push(root);
        }
        return queue.top();
    }

    void free_tree(node* root) {
        std::queue<node*> queue;
        queue.push(root);

        while (!queue.empty()) {
            node* n = queue.front();
            node* left = n->left;
            node* right = n->right;

            queue.pop();

            if (left) {
                queue.push(left);
                queue.push(right);
            }
            delete n;
        }
    }

    uint32_t* get_freqs(std::ifstream& input) {
        uint32_t* freqs = new uint32_t [N_CHARS] ();
        uint32_t input_size = 0;
        unsigned char c;

        while (input.read(reinterpret_cast<char*>(&c), sizeof(char))) {
            ++freqs[c];
            ++input_size;
        }

        std::cout << input_size << std::endl;
        return freqs;
    }

    size_t* sort_indices(std::string const* arr, size_t const n) {
        size_t* indices = new size_t [n] ();
        auto cmp = [&arr](size_t idx_1, size_t idx_2) {
            return arr[idx_1] < arr[idx_2]; 
        };

        for (size_t i = 0; i < n; ++i) {
            indices[i] = i;
        }
        std::sort(indices, indices + n, cmp);
        return indices;
    }
}

void compress(std::ifstream& input, std::ofstream& output, bool const print_stats) {
    uint32_t* freqs = get_freqs(input);
    uint16_t len = 0;

    for (size_t i = 0; i < N_CHARS; ++i) {
        if (freqs[i] > 0) {
            ++len;
        }
    }
    if (len == 0) {
        std::cout << "0\n0" << std::endl;
        delete[] freqs;
        return;
    }
    output.write(reinterpret_cast<const char*>(&len), sizeof(uint16_t));

    node* tree = build_tree(freqs);
    std::string* codes = get_codes(tree);
    free_tree(tree);

    uint32_t data_size = sizeof(uint16_t);
    for (size_t i = 0; i < N_CHARS; ++i) {
        if (freqs[i] > 0) {
            output.write(reinterpret_cast<const char*>(&i), sizeof(char));
            output.write(reinterpret_cast<const char*>(&freqs[i]), sizeof(uint32_t));
            data_size += sizeof(char) + sizeof(uint32_t);
        }
    }
    delete[] freqs;

    input.clear();
    input.seekg(0, std::ios::beg);
        
    encode(input, codes, output);

    std::cout << data_size << std::endl;

    if (print_stats) {
        size_t* indices = sort_indices(codes, N_CHARS);

        for (size_t i = 0; i < N_CHARS; ++i) {
            size_t idx = indices[i];
            if (codes[idx] != "") {
                std::cout << codes[idx] << " " << idx << std::endl;
            }
        }
        delete[] indices;
    }
    delete[] codes;
}

void decompress(std::ifstream& input, std::ofstream& output, bool const print_stats) {
    uint32_t freqs [N_CHARS] = { 0 };
    uint32_t n_symbols = 0;

    uint32_t data_size = 0;

    uint16_t len;
    if (!input.read(reinterpret_cast<char*>(&len), sizeof(uint16_t))) {
        std::cout << "0\n0\n0" << std::endl;
        return;
    }
    data_size += sizeof(uint16_t);

    for (size_t i = 0; i < len; ++i) {
        unsigned char c;
        input.read(reinterpret_cast<char*>(&c), 1);
        input.read(reinterpret_cast<char*>(&freqs[int(c)]), sizeof(uint32_t));
        n_symbols += freqs[int(c)];
        data_size += sizeof(char) + sizeof(uint32_t);
    }

    node* tree = build_tree(freqs);

    decode(n_symbols, input, tree, output);

    std::cout << data_size << std::endl;

    if (print_stats) {
        std::string* codes = get_codes(tree);
        size_t* indices = sort_indices(codes, N_CHARS);

        for (size_t i = 0; i < N_CHARS; ++i) {
            size_t idx = indices[i];
            if (codes[idx] != "") {
                std::cout << codes[idx] << " " << idx << std::endl;
            }
        }
        delete[] indices;
        delete[] codes;
    }

    free_tree(tree);
}
