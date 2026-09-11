#pragma once

#include <meta> // reflection

namespace fjson {

template <class T>
concept is_reflectable_class = requires {
    requires std::meta::is_class_type(^^T);
    requires !std::empty(std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::unprivileged()));
};

} // namespace fjson