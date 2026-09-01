#pragma once

#include <concepts> // concepts
#include <string> // std::string

// concepts
#include "type_traits/is_optional.hpp" // fjson::is_optional
#include "type_traits/is_vector.hpp" // fjson::has_from_json
#include "type_traits/has_from_json.hpp" // fjson::has_from_json
#include "type_traits/is_reflectable.hpp" // fjson::is_reflectable
#include "type_traits/has_annotation.hpp" // fjson::has_serialize_ann

// annotations
#include "annotations/deserializable.hpp" // fjson::deserializable_t

namespace fjson {

class Value;

template <class T>
concept json_deserializable = (
    std::integral<T> ||
    std::floating_point<T> ||
    std::same_as<std::string, T> ||
    std::same_as<std::string_view, T> ||
    is_optional<T> ||
    is_vector<T> ||
    (
        has_annotation<T, deserializable_t> &&
        (
            has_from_json<T> ||
            is_reflectable_class<T> ||
            std::constructible_from<T, const Value&>
        )
    )
);

} // namespace fjson