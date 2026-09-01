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
#include <meta> // reflection
#include <utility>

#include "json_deserializable.hpp" // fjson::serializable
#include "annotations/skip.hpp" // fjson::skipt_t
#include "type_traits/has_annotationed_fields.hpp" // fjson::has_annotationed_fields
#include "type_traits/has_fjson_traits.hpp" // fjson::has_fjson_traits

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
    // Default - null constructor
    constexpr Value() = default;

    // String constructor
    constexpr explicit Value(std::string_view str);
    constexpr explicit Value(const char* str);

    // Number constructor
    template <std::integral T>
        requires (!std::same_as<T, bool>)
    constexpr explicit Value(T number);

    template <std::floating_point T>
    constexpr explicit Value(T number);

    // Bool constructor
    constexpr explicit Value(bool val);

    // Array constructor
    template <class ArrayTp>
        requires (std::same_as<std::remove_cvref_t<ArrayTp>, array_type>)
    constexpr explicit Value(ArrayTp&& array);

    // Object constructor
    template <class ObjectTp>
        requires (std::same_as<std::remove_cvref_t<ObjectTp>, object_type>)
    constexpr explicit Value(ObjectTp&& object);
public:
    std::optional<Value> find_field_by_string(std::string_view target) const;

public:
    // Serialization

    template <json_deserializable T>
    T as() const noexcept;

    template <json_deserializable T>
    constexpr std::optional<T> try_as() const noexcept;

public:
    template <class Self>
    constexpr decltype(auto) get_raw_variant(this Self&& self);
};

constexpr Value::Value(const std::string_view str)
    : data_(std::string{str}) {}

constexpr  Value::Value(const char* str)
    : data_(str) {}

template <std::integral T>
    requires (!std::same_as<T, bool>)
constexpr Value::Value(const T number)
    : data_(number) {}

template <std::floating_point T>
constexpr Value::Value(T number)
    : data_(number) {}

constexpr Value::Value(const bool val)
    : data_(val) {}

template <class ArrayTp>
    requires (std::same_as<std::remove_cvref_t<ArrayTp>, Value::array_type>)
constexpr Value::Value(ArrayTp&& array)
    : data_(std::forward<ArrayTp>(array)) {}

template <class ObjectTp>
    requires (std::same_as<std::remove_cvref_t<ObjectTp>, Value::object_type>)
constexpr Value::Value(ObjectTp&& object)
    : data_(std::forward<ObjectTp>(object)) {}

inline std::optional<Value> Value::find_field_by_string(const std::string_view target) const {
    if (auto* p_obj = std::get_if<object_type>(&data_)) {
        for (const auto& [name, val] : *p_obj) {
            if (name == target) {
                return val;
            }
        }

        return std::nullopt;
    }

    return std::nullopt;
}

template <json_deserializable T>
T Value::as() const noexcept {
    if constexpr (requires { std::get_if<T>(&data_); }) {
        if (auto* result = std::get_if<T>(&data_)) {
            return *result;
        }
    } else {
        static constexpr auto arg_list =
        std::define_static_array(std::meta::template_arguments_of(^^std::decay_t<data_type>));

        template for (constexpr auto arg : arg_list) {
            using CurrTp = [:arg:];

            if constexpr (std::convertible_to<CurrTp, T>) {
                if (auto ptr = std::get_if<CurrTp>(&data_)) {
                    return static_cast<T>(*ptr);
                }
            }
        }
    }
}

template <json_deserializable T>
constexpr std::optional<T> Value::try_as() const noexcept {
    // Priority
    // 1. has T::from_json()
    // 2. has fjson::traits<T>
    // 3. annotations
    // 4. automatic reflection
    if constexpr (has_from_json<T>) {
        return T::from_json(*this);
    }

    if constexpr (has_json_traits<T>) {
        return json_traits<T>::from_json(*this);
    }

    if constexpr (has_annotation<T, deserializable_t>) {
        if constexpr (has_annotationed_field<T>) {
            T obj{};
            static constexpr auto fields =
                std::define_static_array(
                    std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::unprivileged()
                )
            );

            template for (constexpr auto field : fields) {
                constexpr auto skip_anns =
                    std::define_static_array(std::meta::annotations_of_with_type(field, ^^skip_t));

                if constexpr (!skip_anns.empty()) {
                    continue;
                }

                using FieldT = [: std::meta::type_of(field) :];
                constexpr auto field_name = std::meta::identifier_of(field);

                if (auto json_field = find_field_by_string(field_name)) {
                    if (auto val = json_field->template try_as<FieldT>()) {
                        obj.[: field :] = *val;
                    }
                }
            }
            return obj;
        }

        T obj{};
        static constexpr auto fields =
            std::define_static_array(
                std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::unprivileged()
            )
        );

        template for (constexpr auto field : fields) {
            using FieldT = [: std::meta::type_of(field) :];
            constexpr auto field_name = std::meta::identifier_of(field);

            if (auto json_field = find_field_by_string(field_name)) {
                if (auto val = json_field->template try_as<FieldT>()) {
                    obj.[: field :] = *val;
                }
            }
        }

        return obj;
    }

    static constexpr auto types =
        std::define_static_array(std::meta::template_arguments_of(
            ^^std::variant<
                std::monostate, // Null
                std::string, // String
                std::uint64_t, std::int64_t, double, // Number
                bool, // Boolean
                std::vector<Value>, // Array
                std::vector<std::pair<std::string, Value>> // Object
            >)
        );
    template for (constexpr auto type : types) {
        using CurrT = [: type :];

        if constexpr (std::convertible_to<CurrT, T>) {
            if (auto* p_res = std::get_if<CurrT>(&data_)) {
                return static_cast<T>(*p_res);
            }
        }
    }

    return std::nullopt;
}

template <class Self>
constexpr decltype(auto) Value::get_raw_variant(this Self&& self)  {
    return std::forward<Self>(self).data_;
}

} // namespace fjson

