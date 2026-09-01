#pragma once

#include <meta> // reflection
#include <variant> // std::variant
#include <cstdint> // std::uint64_t, std::int64_t

namespace fjson {

class Value;

template <class T>
concept is_json_convertible = requires {
    [] {
        static constexpr auto arg_list =
            std::define_static_array(std::meta::template_arguments_of(
                ^^std::variant<
                    std::monostate, // null
                    std::string, // string
                    std::uint64_t, std::int64_t, double, // Number
                    bool, // Boolean
                    std::vector<Value>, // Array
                    std::vector<std::pair<std::string, Value>> // Object
                >
            )
        );    

        template for (constexpr auto arg : arg_list) {
            using CurrT = [:arg:];

            if constexpr (std::convertible_to<CurrT, T>) {
                return true;
            }
        }

        return false;
    }();
};

} // namespace fjson