# C++26 MODERN HEADER-ONLY LIBRARY FOR JSON 
SERIALIZATION / DESERIALIZATION

**Note:**
only gcc supports some c++26 functionality now

**How to install:**
- cmake minimal example
```cmake
include(FetchContent)
FetchContent_Declare(
        fjson
        GIT_REPOSITORY https://github.com/def1ne0/fjson
        GIT_TAG v0.1.1 # or another version
)
FetchContent_MakeAvailable(fjson)
# ...
target_link_libraries(fjson::fjson)
```

Done:
1. Deserialize support via default deserialization, static from_json method and fjson::json_traits<T> implementation

Goals:
1. JSON serialization (parser) and some new attributes/utility
2. Code refactoring and optimization
3. Tests

Example usage:
```c++
#include <string>
#include <fjson/json.hpp>

struct [[=fjson::deserializable]] Person {
    [[=fjson::skip]]
    int age;
    std::string name;
};

int main() {
    constexpr auto json = fjson::ObjectBuilder{}
        .member("age", 5)
        .member("name3", "hi")
        .collect();

    auto person = json.try_as<Person>();

    assert(person);
    assert(person->age == 0);
    assert(person->name == "hi");
}
```
```c++
#include <string>
#include <fjson/json.hpp>

namespace {

struct [[=fjson::deserializable]] Person {
    int age;
    std::string name;
}

} // namespace

int main() {
    constexpr auto j1 = fjson::Value{"1"};

    try {
        auto r1 = j1.as<Person>();
        std::println("success");
    } catch (const std::runtime_error& err) {
        std::println("{}", err.what());
    }
}
```
```terminaloutput
Cannot deserialize std::__cxx11::basic_string<char> to {anonymous}::Person
```
