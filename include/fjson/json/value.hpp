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
#include <ranges> // std::views::iota, std::unreachable

#include "json_deserializable.hpp" // fjson::serializable
#include "annotations/skip.hpp" // fjson::skipt_t
#include "fjson/detail/deserialize/get_fields_aggregate.hpp"
#include "type_traits/has_annotationed_fields.hpp" // fjson::has_annotationed_fields
#include "type_traits/has_json_traits.hpp" // fjson::has_fjson_traits

namespace fjson {

class Value final {
public:
    using array_type = std::vector<Value>;
    using member_type = std::pair<std::string, Value>;
    using object_type = std::vector<member_type>;
    using data_type = std::variant<
        std::monostate, // Null
        std::string, // String
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
    template <class ArrayT>
        requires (std::same_as<std::remove_cvref_t<ArrayT>, array_type>)
    constexpr explicit Value(ArrayT&& array);

    // Object constructor
    template <class ObjectT>
        requires (std::same_as<std::remove_cvref_t<ObjectT>, object_type>)
    constexpr explicit Value(ObjectT&& object);
public:
    [[nodiscard]] std::optional<Value> find_field_by_string(std::string_view target) const;

public:
    // Serialization
    template <json_deserializable T, class Self>
    T as(this Self&& self);

    template <json_deserializable T, class Self>
    constexpr std::optional<T> try_as(this Self&& self) noexcept;

public:
    template <class Self>
    constexpr decltype(auto) get_raw_variant(this Self&& self);

    // Checks if std::variant holds object_type
    // representing JSON object
    bool is_object() const;
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

template <class ArrayT>
    requires (std::same_as<std::remove_cvref_t<ArrayT>, Value::array_type>)
constexpr Value::Value(ArrayT&& array)
    : data_(std::forward<ArrayT>(array)) {}

template <class ObjectT>
    requires (std::same_as<std::remove_cvref_t<ObjectT>, Value::object_type>)
constexpr Value::Value(ObjectT&& object)
    : data_(std::forward<ObjectT>(object)) {}

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

template <json_deserializable T, class Self>
T Value::as(this Self&& self) {
    const auto res = self.template try_as<T, Self>();

    if (!res) {
        constexpr auto types = std::define_static_array(std::meta::template_arguments_of(std::meta::dealias(^^data_type)));

        std::string variant_curr_type = "UNDEFINED_TYPE";

        template for (constexpr auto i : std::views::iota(0uz, types.size())) {
            if (self.data_.index() == i) {
                variant_curr_type = std::meta::display_string_of(types[i]);
            }
        }

        throw std::runtime_error(std::format("Cannot deserialize {} to {}", variant_curr_type, std::meta::display_string_of(^^T)));
    }

    return *res;
}

template <json_deserializable T, class Self>
constexpr std::optional<T> Value::try_as(this Self&& self) noexcept {
    // Priority
    // 1. has T::from_json()
    // 2. has fjson::traits<T>
    // 3. annotations
    // 4. automatic reflection
    constexpr bool is_rvalue_obj = std::is_rvalue_reference_v<Self>;
    std::optional<T> result;

    if constexpr (has_from_json<T>) {
        return T::from_json(std::forward<Self>(self));
    } else if constexpr (has_json_traits<T>) {
        return json_traits<T>::from_json(std::forward<Self>(self));
    } else if constexpr (has_annotation<T, deserializable_t>) {
        if (!self.is_object()) {
            return result;
        }

        if constexpr (has_annotationed_field<T>) {
            static constexpr auto fields =
                std::define_static_array(
                    std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::unprivileged()
                )
            );

            struct FieldsStorage;

            consteval {
                auto specifiers = __detail::deserialize::extract_storage_specifiers(fields);

                std::meta::define_aggregate(^^FieldsStorage, specifiers);
            }

            FieldsStorage storage;

            static constexpr auto fields_storage = std::define_static_array(std::meta::nonstatic_data_members_of(^^FieldsStorage, std::meta::access_context::unprivileged()));

            template for (constexpr std::size_t i : std::views::iota(0zu, fields.size())) {
                constexpr auto field = fields[i];
                constexpr auto skip_anns =
                    std::define_static_array(std::meta::annotations_of_with_type(field, ^^skip_t));
                using FieldT = [:std::meta::type_of(field):];

                if constexpr (!skip_anns.empty()) {
                    storage.[:fields_storage[i]:] = FieldT{};
                    continue;
                }

                constexpr auto field_name = std::meta::identifier_of(field);

                if (auto json_field = self.find_field_by_string(field_name)) {
                    if (auto val = json_field->template try_as<FieldT, Self>()) {
                        if (std::is_rvalue_reference_v<Self>) {
                            storage.[:fields_storage[i]:] = std::move(*val);
                        } else {
                            storage.[:fields_storage[i]:] = *val;
                        }
                    }
                }
            }

            [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                result.emplace(std::move(storage.[:fields_storage[Is]:])...);
            }(std::make_index_sequence<fields.size()>{});
        } else {
            static constexpr auto fields =
                std::define_static_array(
                    std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::unprivileged()
                )
            );

            struct FieldsStorage;

            consteval {
                auto specifiers = __detail::deserialize::extract_storage_specifiers(fields);

                std::meta::define_aggregate(^^FieldsStorage, specifiers);
            }

            FieldsStorage storage;

            static constexpr auto fields_storage = std::define_static_array(std::meta::nonstatic_data_members_of(^^FieldsStorage, std::meta::access_context::unprivileged()));

            template for (constexpr std::size_t i : std::views::iota(0zu, fields.size())) {
                constexpr auto field = fields[i];
                using FieldT = [:std::meta::type_of(field):];
                constexpr auto field_name = std::meta::identifier_of(field);

                if (auto json_field = self.find_field_by_string(field_name)) {
                    if (auto val = json_field->template try_as<FieldT, Self>()) {
                        if (std::is_rvalue_reference_v<Self>) {
                            storage.[:fields_storage[i]:] = std::move(*val);
                        } else {
                            storage.[:fields_storage[i]:] = *val;
                        }
                    }
                }
            }

            [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                result.emplace(std::move(storage.[:fields_storage[Is]:])...);
            }(std::make_index_sequence<fields.size()>{});
        }
    } else {
        static constexpr auto types =
        std::define_static_array(std::meta::template_arguments_of(
            std::meta::dealias(^^data_type)
        ));

        template for (constexpr auto type : types) {
            using CurrT = [:type:];

            if (auto* p_res = std::get_if<CurrT>(&self.data_)) {
                if constexpr (std::convertible_to<CurrT, T>) {
                    return static_cast<T>(*p_res);
                } else if constexpr (std::constructible_from<T, CurrT>) {
                    if constexpr (is_rvalue_obj) {
                        result.emplace(*p_res);
                    } else {
                        result.emplace(std::move(*p_res));
                    }

                    break;
                }
            }
        }
    }

    return result;
}

template <class Self>
constexpr decltype(auto) Value::get_raw_variant(this Self&& self)  {
    return std::forward<Self>(self).data_;
}

inline bool Value::is_object() const {
    return data_.index() == 7;
}

} // namespace fjson

