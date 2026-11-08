#pragma once

#include <expected>
#include <orchard/dot/base/diagnostic.hpp>
#include <orchard/dot/model/value.hpp>
#include <orchard/dot/syntax/lexer.hpp>
#include <span>

namespace orchard::dot {

inline constexpr int max_nesting_depth = 128;

[[nodiscard]] std::expected<value, diagnostic>
parse(std::span<const token> tokens);

} // namespace orchard::dot
