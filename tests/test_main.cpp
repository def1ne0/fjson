#include <cassert>
#include <format>

#include <fjson/json.hpp>



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
    [[= fjson::skip]]
    int age;
    std::string name;
};

} // namespace

int main() {
    auto json = fjson::ObjectBuilder{}
        .member("age", 5)
        .member("name", "hi")
        .collect();

    auto person = json.try_as<Person>();

    assert(person);
    assert(person->age == 0);
    assert(person->name == "hi");

    auto j1 = fjson::Value{};
    auto r1 = j1.try_as<bool>();

    assert(!r1);

    return 0;
}