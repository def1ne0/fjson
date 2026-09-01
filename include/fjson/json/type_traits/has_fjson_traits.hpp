#pragma once

#include "fjson/json/type_traits/json_traits.hpp"

namespace fjson {

template <class T>
concept has_json_traits = requires(const Value& v, const T& obj) {
    { json_traits<T>::from_json(v) } -> std::same_as<std::optional<T>>;
    { json_traits<T>::to_json(obj) } -> std::same_as<Value>;
};

} // namespace fjson