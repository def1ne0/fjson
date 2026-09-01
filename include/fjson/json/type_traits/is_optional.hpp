#pragma once

#include <optional> // std::optional
#include <type_traits> // std::false_type, std::true_type

namespace fjson {

template <class>
struct is_optional_t : std::false_type {};

template <class T>
struct is_optional_t<std::optional<T>> : std::true_type {};

template <class T>
concept is_optional = is_optional_t<T>::value;

} // namespace fjson
