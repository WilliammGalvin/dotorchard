#pragma once

#include "orchard/dot/base/span.hpp"
#include "orchard/dot/model/value.hpp"
#include <algorithm>
#include <expected>
#include <limits>
#include <orchard/dot/base/diagnostic.hpp>
#include <string>
#include <type_traits>

namespace orchard::dot {

template <class T> struct tag {};

template <class T> using convert_result = std::expected<T, diagnostic>;

template <class T> [[nodiscard]] convert_result<T> from_value(const value &v);

[[nodiscard]] inline diagnostic mismatch_error(value_type expected,
                                               const value &got) {
  return make_error(diag_code::type_mismatch, got.span(),
                    "expected " + std::string(type_name(expected)) +
                        ", found " + std::string(type_name(got.kind())));
}

namespace detail {

template <class I>
concept int_like =
    std::integral<I> && !std::same_as<I, bool> && !std::same_as<I, char> &&
    !std::same_as<I, wchar_t> && !std::same_as<I, char8_t> &&
    !std::same_as<I, char16_t> && !std::same_as<I, char32_t>;

template <class I> std::string range_text() {
  using lim = std::numeric_limits<I>;
  if constexpr (std::is_signed_v<I>)
    return std::to_string(static_cast<long long>(lim::min())) + ".." +
           std::to_string(static_cast<long long>(lim::max()));
  else
    return std::to_string(static_cast<unsigned long long>(lim::min())) + ".." +
           std::to_string(static_cast<unsigned long long>(lim::max()));
}

inline source_span open_span(source_span s) {
  return s.has_file() ? source_span{s.file, s.begin, s.begin + 1} : s;
}

} // namespace detail

} // namespace orchard::dot
