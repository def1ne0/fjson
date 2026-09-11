#pragma once

#include <meta> // reflection
#include <variant> // std::variant

#include "fjson/json/value.hpp" // fjson::Value::data_type

namespace fjson {

class Value;

template <class T>
concept is_json_convertible = requires {
    [] {
        static constexpr auto arg_list =
            std::define_static_array(std::meta::template_arguments_of(
                std::meta::dealias(^^Value::data_type)
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