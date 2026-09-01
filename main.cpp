#include <cassert>
#include <format>
#include <print>
#include <variant>

#include "fjson/json.hpp"
#include "fjson/json/json_deserializable.hpp"
#include "fjson/json/annotations/serializable.hpp"
#include "fjson/json/annotations/skip.hpp"
#include "fjson/json/type_traits/has_annotationed_fields.hpp"

template <>
struct std::formatter<fjson::Value> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const fjson::Value& value, std::format_context& ctx) const  {
        const auto& data = value.get_raw_variant();
        auto out = ctx.out();

        if (auto* p_boolean = std::get_if<bool>(&data)) {
            return std::format_to(out, "{}", *p_boolean);
        }

        if (auto* p_number = std::get_if<std::int64_t>(&data)) {
            return std::format_to(out, "{}", *p_number);
        }

        if (auto* p_number = std::get_if<std::uint64_t>(&data)) {
            return std::format_to(out, "{}", *p_number);
        }

        if (auto* p_number = std::get_if<double>(&data)) {
            return std::format_to(out, "{}", *p_number);
        }

        if (auto* p_str = std::get_if<std::string>(&data)) {
            return std::format_to(out, "\"{}\"", *p_str);
        }

        if (auto* p_array = std::get_if<fjson::Value::array_type>(&data)) {
            out = std::format_to(out, "[ ");
            const auto sz = p_array->size();

            for (auto i = 0uz; i < sz; ++i) {
                out = std::format_to(out, "{}", (*p_array)[i]);

                if (i + 1 < sz) {
                    out = std::format_to(out, ", ");
                }
            }

            return std::format_to(out, " ]");
        }

        if (auto* p_object = std::get_if<fjson::Value::object_type>(&data)) {
            out = std::format_to(out, "{{ ");
            const auto sz = p_object->size();

            for (auto i = 0uz; i < sz; ++i) {
                out = std::format_to(out, "{} : {}", (*p_object)[i].first,  (*p_object)[i].second);

                if (i + 1 < sz) {
                    out = std::format_to(out, ", ");
                }
            }

            return std::format_to(out, " }}");
        }

        if (std::get_if<std::monostate>(&data)) {
            return std::format_to(out, "null");
        }

        return std::format_to(ctx.out(), "Type undefined");

    }
};

namespace {

struct [[= fjson::deserializable]] Person {
    std::int64_t age = 52;
    std::string name = "sigma";
};

} // namespace

int main() {
    fjson::Value json1 = fjson::ArrayBuilder{}
        .item(42)
        .item("some_string")
        .item(true)
        .item(fjson::ObjectBuilder{}
            .member("id", 52)
            .member("pi", 3.14)
            .collect()
        )
        .collect();

    // std::println("{}", json);
    // [ 42, "some_string", true, { id : 52, pi : 3.14 } ]

    auto json = fjson::ObjectBuilder{}
        .member("age", 5)
        .member("name", "Вася")
        .collect();

    auto test = fjson::Value{5};
    auto res = test.try_as<bool>();
    assert(res);
    assert(*res);
    return 0;
}


