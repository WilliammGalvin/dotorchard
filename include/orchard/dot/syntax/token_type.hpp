#pragma once

#include <cstdint>
#include <string_view>

namespace orchard::dot {

enum class token_type : std::uint8_t {
  identifier,

  integer_literal,
  float_literal,
  string_literal,
  true_literal,
  false_literal,

  equals,
  semi_colon,
  comma,
  left_brace,
  right_brace,
  left_bracket,
  right_bracket,

  unknown,
  eof,
};

inline constexpr std::string_view to_str(token_type type) noexcept {
  switch (type) {
  case token_type::identifier:
    return "identifier";

  case token_type::integer_literal:
    return "integer_literal";
  case token_type::float_literal:
    return "float_literal";
  case token_type::string_literal:
    return "string_literal";
  case token_type::true_literal:
    return "true_literal";
  case token_type::false_literal:
    return "false_literal";

  case token_type::equals:
    return "equals";
  case token_type::semi_colon:
    return "semi_colon";
  case token_type::comma:
    return "comma";
  case token_type::left_brace:
    return "left_brace";
  case token_type::right_brace:
    return "right_brace";
  case token_type::left_bracket:
    return "left_bracket";
  case token_type::right_bracket:
    return "right_bracket";

  case token_type::eof:
    return "eof";
  case token_type::unknown:
    return "unknown";
  }

  return {};
}

} // namespace orchard::dot
