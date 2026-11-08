#include <algorithm>
#include <orchard/dot/base/diagnostic.hpp>
#include <utility>

namespace orchard::dot {

std::string_view code_name(diag_code code) noexcept {
  switch (code) {
  case diag_code::io_failure:
    return "io_failure";

  case diag_code::unexpected_character:
    return "unexpected_character";
  case diag_code::unterminated_string:
    return "unterminated_string";
  case diag_code::newline_in_string:
    return "newline_in_string";
  case diag_code::invalid_escape:
    return "invalid_escape";
  case diag_code::malformed_number:
    return "malformed_number";

  case diag_code::unexpected_token:
    return "unexpected_token";
  case diag_code::expected_semicolon:
    return "expected_semicolon";
  case diag_code::expected_value:
    return "expected_value";
  case diag_code::duplicate_key:
    return "duplicate_key";
  case diag_code::integer_overflow:
    return "integer_overflow";
  case diag_code::float_out_of_range:
    return "float out of range";
  case diag_code::nesting_too_deep:
    return "nesting too deep";

  case diag_code::type_mismatch:
    return "type_mismatch";
  case diag_code::missing_key:
    return "missing_key";
  case diag_code::unknown_key:
    return "unknown_key";
  case diag_code::out_of_range:
    return "out_of_range";
  }

  return "unknown";
}

std::string_view default_message(diag_code code) noexcept {
  switch (code) {
  case diag_code::io_failure:
    return "I/O error";

  case diag_code::unexpected_character:
    return "unexpected character";
  case diag_code::unterminated_string:
    return "unterminated string literal";
  case diag_code::newline_in_string:
    return "newline in string literal";
  case diag_code::invalid_escape:
    return "invalid escape sequence";
  case diag_code::malformed_number:
    return "malformed number";

  case diag_code::unexpected_token:
    return "unexpected token";
  case diag_code::expected_semicolon:
    return "expected ';'";
  case diag_code::expected_value:
    return "expected a value";
  case diag_code::duplicate_key:
    return "duplicate key";
  case diag_code::integer_overflow:
    return "integer literal out of range";
  case diag_code::float_out_of_range:
    return "floating point literal out of range";
  case diag_code::nesting_too_deep:
    return "nesting too deep";

  case diag_code::type_mismatch:
    return "value has the wrong type";
  case diag_code::missing_key:
    return "missing required key";
  case diag_code::unknown_key:
    return "unknown key";
  case diag_code::out_of_range:
    return "value out of range";
  }

  return "error";
}

diagnostic make_error(diag_code code, source_span span) {
  return make_error(code, span, std::string(default_message(code)));
}

diagnostic make_error(diag_code code, source_span span, std::string message) {
  return diagnostic{severity::error, code, span, std::move(message), {}};
}

diagnostic make_io_error(const file_path &path, io_error error) {
  return make_error(diag_code::io_failure, no_span,
                    path.string() + ": " + std::string(to_string(error)));
}

bool has_errors(std::span<const diagnostic> list) noexcept {
  return std::ranges::any_of(
      list, [](const diagnostic &d) { return d.level == severity::error; });
}

namespace {

constexpr std::string_view severity_name(severity s) noexcept {
  switch (s) {
  case severity::error:
    return "error";
  case severity::warning:
    return "warning";
  case severity::note:
    return "note";
  }

  return "error";
}

void append_header(std::string &out, severity sev, std::string_view message,
                   source_span span, const source_map &map,
                   std::string_view code = {}) {
  if (span.has_file()) {
    const auto &file = map.get(span.file);
    const auto lc = file.locate(span.begin);
    out += file.name();
    out += ':';
    out += std::to_string(lc.line);
    out += ':';
    out += std::to_string(lc.column);
    out += ": ";
  }

  out += severity_name(sev);
  out += ": ";
  out += message;

  if (!code.empty()) {
    out += " [";
    out += code;
    out += ']';
  }

  out += '\n';
}

void append_snippet(std::string &out, source_span span, const source_map &map) {
  if (!span.has_file())
    return;

  const auto &file = map.get(span.file);
  const auto lc = file.locate(span.begin);
  const auto line = file.line_text(lc.line);
  const auto number = std::to_string(lc.line);

  const std::size_t col0 = std::min<std::size_t>(lc.column - 1, line.size());
  std::size_t len = std::max<std::uint32_t>(span.size(), 1);
  len = col0 < line.size() ? std::min(len, line.size() - col0) : 1;

  out += ' ';
  out += number;
  out += " | ";
  out += line;
  out += '\n';

  out += std::string(number.size() + 1, ' ');
  out += " | ";
  for (std::size_t i = 0; i < col0; ++i)
    out += line[i] == '\t' ? '\t' : ' ';

  out += std::string(len, '^');
  out += '\n';
}

} // namespace

std::string render(const diagnostic &d, const source_map &map) {
  std::string out;
  append_header(out, d.level, d.message, d.span, map, code_name(d.code));
  append_snippet(out, d.span, map);

  for (const auto &note : d.notes) {
    append_header(out, severity::note, note.message, note.span, map);
    append_snippet(out, note.span, map);
  }

  return out;
}

std::string render_all(std::span<const diagnostic> list,
                       const source_map &map) {
  std::string out;
  for (const auto &d : list) {
    if (!out.empty())
      out += '\n';
    out += render(d, map);
  }

  return out;
}

} // namespace orchard::dot
