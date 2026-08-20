#include <format>
#include <variant>
#include <print>

#include "include/fjson/json.hpp"

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

int main() {
    fjson::Value json = fjson::ArrayBuilder{}
        .item(42)
        .item("some_string")
        .item(true)
        .item(fjson::ObjectBuilder{}
            .member("id", 52)
            .member("pi", 3.14)
            .collect()
        )
        .collect();

    auto v = fjson::ObjectBuilder{}
        .member("id", 42)
        .member("tags",
            fjson::ArrayBuilder{}
                .item("cpp")
                .collect()
        )
        .collect();

    std::println("{}", json);
    // [ 42, "some_string", true, { id : 52, pi : 3.14 } ]
    std::println("{}", v);

    return 0;
}


