#pragma once

#include <optional> // std::optional

namespace fjson {

class Value;

template <class T>
concept has_from_json = requires(const Value& value) {
      { T::from_json(value) } -> std::same_as<std::optional<T>>;
};

} // namespace fjson
