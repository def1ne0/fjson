#pragma once

// Example usage
//
// fjson::Value = fjson::ObjectBuilder{}
//     .member("pi", 3.14)
//     .member("isMath", true)
//     .collect();

#include "value.hpp" // fjson::Value

namespace fjson {

class ObjectBuilder final {
public:
    using member_type = Value::member_type;
    using object_type = Value::object_type;

public:
    template <class Self, class Tp>
    ObjectBuilder&& member(this Self&& self, std::string_view str, Tp&& into_value);

    template <class Self>
    decltype(auto) collect(this Self&& self);
private:
    object_type object_;
};

template <class Self, class Tp>
ObjectBuilder&& ObjectBuilder::member(this Self&& self, std::string_view str, Tp&& into_value) {
    self.object_.emplace_back(member_type{
        str,
        std::forward<Tp>(into_value)
    });

    return std::forward<Self>(self);
}

template <class Self>
decltype(auto) ObjectBuilder::collect(this Self&& self)  {
    return Value{std::forward<Self>(self).object_};
}

} // namespace fjson