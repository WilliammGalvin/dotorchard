#pragma once

#include <cstdint>
#include <orchard/dot/base/file_io.hpp>
#include <orchard/dot/base/source_file.hpp>
#include <orchard/dot/base/span.hpp>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace orchard::dot {

class source_map;

enum class severity : std::uint8_t { error, warning, note };

enum class diag_code : std::uint16_t {
  // io
  io_failure,

  // lexer
  unexpected_character,
  unterminated_string,
  newline_in_string,
  invalid_escape,
  malformed_number,

  // parser
  unexpected_token,
  expected_semicolon,
  expected_value,
  duplicate_key,
  integer_overflow,
  float_out_of_range,
  nesting_too_deep,

  // validator
  type_mismatch,
  missing_key,
  unknown_key,
  out_of_range,
  not_one_of,
  invalid_schema,

  // writer
  invalid_key,
  non_finite_float,
  root_not_table,
};

struct diagnostic_note {
  source_span span;
  std::string message;
};

struct diagnostic {
  severity level{severity::error};
  diag_code code{};
  source_span span{};
  std::string message;
  std::vector<diagnostic_note> notes;

  [[nodiscard]] diagnostic with_note(source_span note_span,
                                     std::string note_message) && {
    notes.push_back({note_span, std::move(note_message)});
    return std::move(*this);
  }
};

using diagnostics = std::vector<diagnostic>;

[[nodiscard]] std::string_view code_name(diag_code code) noexcept;
[[nodiscard]] std::string_view default_message(diag_code code) noexcept;

[[nodiscard]] diagnostic make_error(diag_code code, source_span span);

[[nodiscard]] diagnostic make_error(diag_code code, source_span span,
                                    std::string message);

[[nodiscard]] diagnostic make_io_error(const file_path &path, io_error error);

[[nodiscard]] bool has_errors(std::span<const diagnostic> list) noexcept;

[[nodiscard]] std::string render(const diagnostic &d, const source_map &map);

[[nodiscard]] std::string render_all(std::span<const diagnostic> list,
                                     const source_map &map);

} // namespace orchard::dot
