#pragma once

#include <meta> // reflection

namespace fjson {

template <class T>
concept is_reflectable_class = requires {
    std::meta::is_class_type(^^T),
    std::meta::members_of(^^T, std::meta::access_context::unprivileged());
};

} // namespace fjson