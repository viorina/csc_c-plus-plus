#pragma once

#include "dict.hpp"

#include <map>
#include <string>
#include <type_traits>
#include <vector>

namespace utils {
    template<typename T>
    void write(dict_t& dict, std::vector<T> const& obj) {
        size_t id = 0;
        if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, std::string>) {
            for (T const& value : obj) {
                put(dict, std::to_string(id), value);
                ++id;
            }
        } else {
            for (T const& value : obj) {
                dict_t tmp;
                write(tmp, value);
                put(dict, std::to_string(id), tmp);
                ++id;
            }
        }
    }

    template<typename T>
    void write(dict_t& dict, std::map<std::string, T> const& obj) {
        if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, std::string>) {
            for (auto const& [key, value] : obj) {
                put(dict, std::string(key), value);
            }
        } else {
            for (auto const& [key, value] : obj) {
                dict_t tmp;
                write(tmp, value);
                put(dict, std::string(key), tmp);
            }
        }
    }

    template<typename T>
    void read(dict_t const& dict, std::vector<T>& obj) {
        obj.reserve(dict.size());
        if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, std::string>) {
            for (size_t id = 0; id < dict.size(); ++id) {
                obj.push_back(get<T>(dict, std::to_string(id)));
            }
        } else {
            for (size_t id = 0; id < dict.size(); ++id) {
                T tmp;
                read(get<dict_t>(dict, std::to_string(id)), tmp);
                obj.push_back(tmp);
            }
        }
    }

    template<typename T>
    void read(dict_t const& dict, std::map<std::string, T>& obj) {
        if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, std::string>) {
            for (auto const& [key, value] : dict) {
                obj.insert(std::make_pair(key, get<T>(dict, key)));
            }
        } else {
            for (auto const& [key, value] : dict) {
                T tmp;
                read(get<dict_t>(dict, key), tmp);
                obj.insert(std::make_pair(key, tmp));
            }
        }
    }
}
