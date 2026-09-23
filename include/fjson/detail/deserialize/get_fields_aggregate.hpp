#pragma once
#include <meta>

namespace fjson::_detail_::deserialize {

consteval auto extract_storage_specifiers(
    const std::span<const std::meta::info> fields)
-> std::vector<std::meta::info> {
    auto storage_specifiers = std::vector<std::meta::info>{};

    for (const auto field : fields) {
        storage_specifiers.push_back(
            std::meta::data_member_spec(
                std::meta::type_of(field),
                {.name = std::meta::identifier_of(field)}
            )
        );
    }

    return storage_specifiers;
}

} // namespace fjson::_detail_::deserialize