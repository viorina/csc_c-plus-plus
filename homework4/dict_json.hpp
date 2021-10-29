#pragma once

#include <iostream>
#include <optional>

#include "nlohmann/json.hpp"
#include "dict.hpp"

using json = nlohmann::json;

inline std::optional<int> get_int(utils::whatever const& value) {
    if (value.empty()) {
        return std::nullopt;
    }
    if (value.type_info() == typeid(int)) {
        return utils::whatever_cast<int>(value);
    }
    if (value.type_info() == typeid(char)) {
        return static_cast<int>(utils::whatever_cast<char>(value));
    }
    if (value.type_info() == typeid(short)) {
        return static_cast<int>(utils::whatever_cast<short>(value));
    }
    if (value.type_info() == typeid(long)) {
        return static_cast<int>(utils::whatever_cast<long>(value));
    }
    if (value.type_info() == typeid(unsigned char)) {
        return static_cast<int>(utils::whatever_cast<unsigned char>(value));
    }
    if (value.type_info() == typeid(unsigned int)) {
        return static_cast<int>(utils::whatever_cast<unsigned int>(value));
    }
    if (value.type_info() == typeid(unsigned short)) {
        return static_cast<int>(utils::whatever_cast<unsigned short>(value));
    }
    if (value.type_info() == typeid(unsigned long)) {
        return static_cast<int>(utils::whatever_cast<unsigned long>(value));
    }
    return std::nullopt;
}

inline json json_from_dict(utils::dict_t const& dict) {
    json obj;
    for (auto const& [key, value] : dict) {
        auto int_value = get_int(value);
        if (int_value != std::nullopt) {
            obj[key] = int_value.value();
        } else if (utils::get_ptr<std::string>(dict, key)) {
            obj[key] = utils::whatever_cast<std::string>(value);
        } else if (utils::get_ptr<double>(dict, key)) {
            obj[key] = utils::whatever_cast<double>(value);
        } else if (utils::get_ptr<bool>(dict, key)) {
            obj[key] = utils::whatever_cast<bool>(value);
        } else if (utils::is_dict(dict, key)) {
            obj[key] = json_from_dict(utils::whatever_cast<utils::dict_t>(value));
        } else {
            obj[key] = {};
        }
    }
    return obj;
}

inline void json_to_dict(json& obj, utils::dict_t& dict) {
    for (auto& [key, value] : obj.items()) {
        if (value.is_boolean()) {
            utils::put(dict, std::string(key), value.get<bool>());
        } else if (value.is_string()) {
            utils::put(dict, std::string(key), value.get<std::string>());
        } else if (value.is_number_integer()) {
            utils::put(dict, std::string(key), value.get<int>());
        } else if (value.is_number_float()) {
            utils::put(dict, std::string(key), value.get<double>());
        } else if (value.is_object()) {
            utils::dict_t tmp;
            json_to_dict(value, tmp);
            utils::put(dict, std::string(key), tmp);
        } else {
            utils::put(dict, std::string(key), utils::whatever());
        }
    }
}

namespace utils {
    inline void save_to_json(std::ostream& os, dict_t const& dict) {
        os << json_from_dict(dict);
    }

    inline bool load_from_json(std::istream& is, dict_t& dict) {
        json obj = json::parse(is);
        if (!obj.is_object()) {
            return false;
        }
        json_to_dict(obj, dict);
        return true;
    }
}
