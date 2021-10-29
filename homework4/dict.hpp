#pragma once

#include "whatever.hpp"

namespace utils {
    typedef std::unordered_map<std::string, whatever> dict_t;

    struct no_key_exception : public std::exception {
        char const* what() const noexcept override {
            return "There is no such key in the dictionary.";
        }
    };

    struct invalid_type_exception : public std::exception {
        char const* what() const noexcept override {
            return "Invalid type of value.";
        }
    };

    template<typename T>
    bool put(dict_t& dict, std::string const& key, T&& value) {
        auto [it, result] = dict.try_emplace(std::move(key), whatever(std::forward<T>(value)));
        return result;
    }

    template<typename T>
    T const* get_ptr(dict_t const& dict, std::string const& key) {
        auto it = dict.find(key);
        if (it == dict.end()) {
            return nullptr;
        }

        try {
            return whatever_cast<T const>(&it->second);
        } catch (bad_whatever_cast const&) {
            return nullptr;
        }
    }

    template<typename T>
    T* get_ptr(dict_t& dict, std::string const& key) {
        auto it = dict.find(key);
        if (it == dict.end()) {
            return nullptr;
        }

        try {
            return whatever_cast<T>(&it->second);
        } catch (bad_whatever_cast const&) {
            return nullptr;
        }
    }

    template<typename T>
    T const& get(dict_t const& dict, std::string const& key) {
        auto it = dict.find(key);
        if (it == dict.end()) {
            throw no_key_exception();
        }

        try {
            return whatever_cast<T const&>(it->second);
        } catch (bad_whatever_cast const& error) {
            throw invalid_type_exception();
        }
    }

    template<typename T>
    T& get(dict_t& dict, std::string const& key) {
        auto it = dict.find(key);
        if (it == dict.end()) {
            throw no_key_exception();
        }

        try {
            return whatever_cast<T&>(it->second);
        } catch (bad_whatever_cast const& error) {
            throw invalid_type_exception();
        }
    }

    inline bool remove(dict_t& dict, std::string const& key) {
        return dict.erase(key) != 0;
    }

    inline bool contains(dict_t const& dict, std::string const& key) {
        return dict.find(key) != dict.end();
    }

    inline bool empty(dict_t const& dict) {
        return dict.empty();
    }

    inline void clear(dict_t& dict) {
        dict.clear();
    }

    inline bool is_dict(dict_t const& dict, std::string const& key) {
        return get_ptr<dict_t>(dict, key) != nullptr;
    }
}
