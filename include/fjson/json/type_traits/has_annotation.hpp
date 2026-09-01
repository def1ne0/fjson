#pragma once

#include <meta> // reflection

namespace fjson {

template <class T, class AnnotationT>
concept has_annotation = !std::ranges::empty(
    std::meta::annotations_of_with_type(^^T, ^^AnnotationT)
);

} // namespace fjson