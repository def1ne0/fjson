#pragma once

#include <meta>

namespace fjson {

template <class T>
concept has_annotationed_field = [] {
    static constexpr auto fields =
        std::define_static_array(std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::unprivileged()));

    template for (constexpr auto field : fields) {
        if (!std::ranges::empty(std::meta::annotations_of(field))) {
            return true;
        }
    }

    return false;
}();

} // namespace fjson