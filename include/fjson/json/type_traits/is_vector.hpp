#pragma once

#include <vector> // std::vector
#include <type_traits> // std::false_type, std::true_type

namespace fjson {

template <class>
struct is_vector_t : std::false_type {};

template <class T>
struct is_vector_t<std::vector<T>> : std::true_type {};

template <class T>
concept is_vector = is_vector_t<T>::value;

} // namespace fjson