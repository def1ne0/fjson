#pragma once

// Example usage
//
// fjson::Value = fjson::ArrayBuilder{}
//     .item("cpp")
//     .item(42)
//     .collect();

#include "value.hpp" // fjson::Value

namespace fjson {

class ArrayBuilder final {
public:
    using array_type = Value::array_type;

public:
    template <class Self, class Tp>
    Self&& item(this Self&& self, Tp&& into_value);

    template <class Self>
    decltype(auto) collect(this Self&& self);

private:
    array_type array_;
};

template <class Self, class Tp>
Self&& ArrayBuilder::item(this Self&& self, Tp&& into_value)  {
    self.array_.emplace_back(std::forward<Tp>(into_value));
    return std::forward<Self>(self);
}

template <class Self>
decltype(auto) ArrayBuilder::collect(this Self&& self) {
    return Value{std::forward<Self>(self).array_};
}

} // namespace fjson