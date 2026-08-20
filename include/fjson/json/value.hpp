#pragma once

// Example usage:
//
// fjson::Value = fjson::ObjectBuilder{}
//      .member("id", 42)
//      .member("tags",
//          fjson::ArrayBuilder{}
//              .item("cpp")
//              .collect()
//       )
//      .collect();

#include <variant> // std::variant
#include <string> // std::string
#include <cstdint> // std::uint64_t
#include <vector> // std::vector
#include <meta>

namespace fjson {
class Value final {
public:
    using array_type = std::vector<Value>;
    using member_type = std::pair<std::string, Value>;
    using object_type = std::vector<member_type>;
    using data_type = std::variant<
        std::monostate, // null
        std::string, // string
        std::uint64_t, std::int64_t, double, // Number
        bool, // Boolean
        array_type, // Array
        object_type // Object
    >;

private:
    data_type data_{};

public:
    // String constructor
    explicit Value(std::string_view str);
    explicit Value(const char* str);

    // Number constructor
    template <std::integral Tp>
        requires (!std::same_as<Tp, bool>)
    explicit Value(Tp number);

    template <std::floating_point Tp>
    explicit Value(Tp number);

    // Bool constructor
    explicit Value(bool val);

    // Array constructor
    template <class ArrayTp>
        requires (std::same_as<std::remove_cvref_t<ArrayTp>, array_type>)
    explicit Value(ArrayTp&& array);

    // Object constructor
    template <class ObjectTp>
        requires (std::same_as<std::remove_cvref_t<ObjectTp>, object_type>)
    explicit Value(ObjectTp&& object);
public:
    // Serialization
    template <class Tp>
    Tp as() const noexcept(false);

    template <class Tp>
    std::optional<Tp> try_as() const noexcept;

public:
    template <class Self>
    decltype(auto) get_raw_variant(this Self&& self);
};

inline Value::Value(const std::string_view str)
    : data_(std::string{str}) {}

inline Value::Value(const char* str)
    : data_(str) {}

template <std::integral Tp>
    requires (!std::same_as<Tp, bool>)
Value::Value(const Tp number)
    : data_(number) {}

template <std::floating_point Tp>
Value::Value(Tp number)
    : data_(number) {}

inline Value::Value(const bool val)
    : data_(val) {}

template <class ArrayTp>
    requires (std::same_as<std::remove_cvref_t<ArrayTp>, Value::array_type>)
Value::Value(ArrayTp&& array)
    : data_(std::forward<ArrayTp>(array)) {}

template <class ObjectTp>
    requires (std::same_as<std::remove_cvref_t<ObjectTp>, Value::object_type>)
Value::Value(ObjectTp&& object)
    : data_(std::forward<ObjectTp>(object)) {}

template <class Tp>
Tp Value::as() const noexcept(false) {
    auto res = try_as<Tp>();

    return res ? *res : throw std::runtime_error(std::format("cannot convert to {}", std::meta::display_string_of(^^Tp)));
}

template <class Tp>
std::optional<Tp> Value::try_as() const noexcept {
    if constexpr (requires { std::get_if<Tp>(&data_); }) {
        if (auto* result = std::get_if<Tp>(&data_)) {
            return *result;
        }
    }

    static constexpr auto arg_list =
        std::define_static_array(std::meta::template_arguments_of(^^std::decay_t<data_type>));


    template for (constexpr auto arg : arg_list) {
        using CurrTp = [:arg:];

        if constexpr (std::convertible_to<CurrTp, Tp>) {
            if (auto ptr = std::get_if<CurrTp>(&data_)) {
                return static_cast<Tp>(*ptr);
            }
        }
    }

    return std::nullopt;
}

template <class Self>
decltype(auto) Value::get_raw_variant(this Self&& self)  {
    return std::forward<Self>(self).data_;
}

} // namespace fjson

