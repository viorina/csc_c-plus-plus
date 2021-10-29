// bignum.hpp - header-only solution goes into this file.
#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace mp {
    ///////////////// BIGNUM CLASS

    struct bignum {
        public:
            bignum();
            bignum(uint32_t n);
            explicit bignum(std::string const& src);
            bignum(bignum const& other);
            bignum& operator=(bignum other);
            explicit operator uint32_t() const;
            explicit operator bool() const;
            std::string to_string() const;

            bignum& operator+=(bignum const& other);
            bignum& operator-=(bignum const& other) = delete;
            bignum& operator*=(bignum const& other);

            void swap(bignum& rhs);

        private:
            std::vector<uint32_t> bits_;
            static const uint64_t BASE = static_cast<uint64_t>(UINT32_MAX) + 1;
    };

    inline std::ostream& operator<<(std::ostream& os, bignum const& n);
    inline std::istream& operator>>(std::istream& is, bignum& n);

    inline bignum operator+(bignum lhs, bignum const& rhs);
    inline bignum operator*(bignum lhs, bignum const& rhs);

    inline bignum::bignum() : bits_(1, 0) {}

    inline bignum::bignum(uint32_t n) : bits_(1, n) {}

    inline bignum::bignum(std::string const& src) : bignum() {
        for (auto c : src) {
            (*this) = (*this) * 10 + (c - '0');
        }
    }

    inline bignum::bignum(bignum const& other) : bits_(other.bits_) {}

    inline bignum& bignum::operator=(bignum other) {
        swap(other);
        return *this;
    }

    inline bignum::operator uint32_t() const {
        return bits_[0];
    }

    inline bignum::operator bool() const {
        return (bits_.size() > 1 || bits_[0] > 0);
    }

    inline std::string bignum::to_string() const {
        if (!*this) {
            return "0";
        }
        std::string result;
        bignum copied(*this);
        while (static_cast<unsigned>(std::count(copied.bits_.begin(), copied.bits_.end(), 0)) != copied.bits_.size()) {
            uint32_t d = 0;
            uint32_t r = 0;
            for (auto it = copied.bits_.end() - 1; it >= copied.bits_.begin(); --it) {
                d = (*it + r * BASE) / 10;
                r = (*it + r * BASE) % 10;
                *it = d; 
            }
            result.push_back(r + '0');
        }
        std::reverse(result.begin(), result.end());
        return result;
    }

    inline bignum& bignum::operator+=(bignum const& other) {
        short carry = 0;
        for (size_t i = 0; i < std::max(bits_.size(), other.bits_.size()) || carry != 0; ++i) {
            if (i == bits_.size()) {
                bits_.push_back(0);
            }
            uint64_t cur = static_cast<uint64_t>(bits_[i]) 
                + (i < other.bits_.size() ? other.bits_[i] : 0) + carry;
            if (cur >= BASE) {
                carry = 1;
                bits_[i] = cur - BASE;
            } else {
                carry = 0;
                bits_[i] = cur;
            }
        }
        return *this;
    }

    inline bignum& bignum::operator*=(bignum const& other) {
        bignum result;
        result.bits_.resize(bits_.size() + other.bits_.size());
        for (size_t i = 0; i < bits_.size(); ++i) {
            uint32_t carry = 0;
            for (size_t j = 0; j < other.bits_.size() || carry != 0; ++j) {
                uint64_t cur = result.bits_[i + j] 
                    + static_cast<uint64_t>(bits_[i]) * (j < other.bits_.size() ? other.bits_[j] : 0) 
                    + carry;
                result.bits_[i + j] = static_cast<uint32_t>(cur % BASE);
                carry = static_cast<uint32_t>(cur / BASE);
            }
        }
        while (result.bits_.size() > 1 && result.bits_.back() == 0) {
            result.bits_.pop_back();
        }
        return (*this = result);
    }

    inline void bignum::swap(bignum& rhs) {
        std::swap(bits_, rhs.bits_);
    }

    std::ostream& operator<<(std::ostream& os, bignum const& n) {
        return os << n.to_string();
    }

    std::istream& operator>>(std::istream& is, bignum& n) {
        std::string src;
        is >> src;
        n = bignum(src);
        return is;
    }

    bignum operator+(bignum lhs, bignum const& rhs) {
        return lhs += rhs;
    }

    bignum operator*(bignum lhs, bignum const& rhs) {
        return lhs *= rhs;
    }

    ///////////////// POLYNOMIAL CLASS

    struct polynomial {
        public:
            explicit polynomial(std::string const& src);
            uint32_t at(size_t idx) const;
            uint32_t& at(size_t idx);
            template<typename T> T operator()(T const& point) const;
        
        private:
            std::vector<uint32_t> coeffs_;
    };

    inline polynomial::polynomial(std::string const& src) {
        std::vector<std::string> tokens;
        size_t pos_start = 0;
        size_t pos_end = 0;
        while ((pos_end = src.find_first_of("^+", pos_start)) != std::string::npos) {
            std::string token = src.substr(pos_start, pos_end - pos_start);
            pos_start = ++pos_end;
            tokens.push_back(token);
        }
        tokens.push_back(src.substr(pos_start));

        uint32_t max_pow = std::strtoul(tokens[1].c_str(), nullptr, 10);
        coeffs_.resize(max_pow + 1, 0);

        for (size_t i = 0; i < tokens.size(); i += 2) {
            uint32_t coeff = std::strtoul(tokens[i].c_str(), nullptr, 10);
            uint32_t pow = std::strtoul(tokens[i + 1].c_str(), nullptr, 10);
            coeffs_[pow] = coeff;
        }
    }

    inline uint32_t polynomial::at(size_t idx) const {
        return (idx >= coeffs_.size() ? 0 : coeffs_[idx]);
    }

    inline uint32_t& polynomial::at(size_t idx) {
        if (idx >= coeffs_.size()) {
            coeffs_.resize(idx + 1, 0);
        }
        return coeffs_[idx];
    }

    template<typename T> T polynomial::operator()(T const& point) const {
        T result = coeffs_[coeffs_.size() - 1];
        for (size_t i = coeffs_.size() - 1; i > 0; --i) {
            result = result * point + coeffs_[i - 1];
        }
        return result; 
    }
}
