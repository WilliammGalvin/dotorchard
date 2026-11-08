#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <orchard/dot/base/diagnostic.hpp>
#include <orchard/dot/base/span.hpp>
#include <orchard/dot/syntax/token_type.hpp>
#include <string_view>
#include <vector>

namespace orchard::dot {

struct token {
  token_type type;
  std::string_view content;
  source_span span;
};

class lexer {
public:
  using token_list = std::vector<token>;

  explicit lexer(std::string_view source, file_id file = 0) noexcept
      : _source(source), _file(file) {}

  [[nodiscard]] std::expected<token_list, diagnostic> lex();

private:
  using lex_result = std::expected<token, diagnostic>;

  std::string_view _source;
  file_id _file;
  std::size_t _pos{0};
  token_list _tokens{};

  [[nodiscard]] bool _at_end() const noexcept { return _pos >= _source.size(); }

  [[nodiscard]] char _peek(std::size_t ahead = 0) const noexcept;

  void _advance(std::size_t count = 1) noexcept {
    _pos = std::min(_pos + count, _source.size());
  }

  void _skip_whitespace_and_comments() noexcept;

  template <typename Pred>
    requires std::predicate<Pred, char>
  void _advance_while(Pred pred) noexcept(noexcept(pred(char{}))) {
    while (!_at_end() && pred(_peek()))
      _advance();
  }

  [[nodiscard]] source_span _span(std::size_t begin,
                                  std::size_t end) const noexcept {
    return {_file, static_cast<std::uint32_t>(begin),
            static_cast<std::uint32_t>(end)};
  }

  [[nodiscard]] token _make(token_type type, std::size_t begin) const noexcept {
    return {type, _source.substr(begin, _pos - begin), _span(begin, _pos)};
  }

  [[nodiscard]] std::unexpected<diagnostic>
  _fail(diag_code code, std::size_t begin, std::size_t end) const {
    return std::unexpected(make_error(code, _span(begin, end)));
  }

  lex_result _lex_next();
  lex_result _lex_string();
  lex_result _lex_number();
  lex_result _lex_symbol();
  lex_result _lex_ident();
};

} // namespace orchard::dot
