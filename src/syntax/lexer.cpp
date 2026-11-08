#include <algorithm>
#include <orchard/dot/syntax/lexer.hpp>
#include <utility>

namespace orchard::dot {

namespace {

constexpr bool is_digit(char c) { return c >= '0' && c <= '9'; }

constexpr bool is_alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

constexpr bool is_space(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

constexpr bool is_quote(char c) { return c == '"' || c == '\''; }

constexpr bool is_ident_start(char c) { return is_alpha(c) || c == '_'; }

constexpr bool is_ident_continue(char c) {
  return is_ident_start(c) || is_digit(c);
}

constexpr bool is_sign(char c) { return c == '-' || c == '+'; }

constexpr bool is_escape(char c) {
  switch (c) {
  case 'n':
  case 't':
  case 'r':
  case '0':
  case '\\':
  case '"':
  case '\'':
    return true;

  default:
    return false;
  }
}

} // namespace

std::expected<lexer::token_list, diagnostic> lexer::lex() {
  while (true) {
    _skip_whitespace_and_comments();
    if (_at_end())
      break;

    auto tok = _lex_next();
    if (!tok)
      return std::unexpected(std::move(tok.error()));

    _tokens.push_back(*tok);
  }

  _tokens.push_back(_make(token_type::eof, _pos)); // empty span at end
  return std::move(_tokens);
}

char lexer::_peek(std::size_t ahead) const noexcept {
  const auto i = _pos + ahead;
  return i < _source.size() ? _source[i] : '\0';
}

void lexer::_skip_whitespace_and_comments() noexcept {
  while (!_at_end()) {
    const char c = _peek();
    if (c == '#')
      _advance_while([](char ch) { return ch != '\n'; });
    else if (is_space(c))
      _advance();
    else
      break;
  }
}

lexer::lex_result lexer::_lex_next() {
  const char c = _peek();
  if (is_quote(c))
    return _lex_string();

  if (is_digit(c) || (is_sign(c) && is_digit(_peek(1))))
    return _lex_number();

  if (is_ident_start(c))
    return _lex_ident();

  return _lex_symbol();
}

lexer::lex_result lexer::_lex_symbol() {
  const auto begin = _pos;

  token_type type;
  switch (_peek()) {
  case '=':
    type = token_type::equals;
    break;
  case ';':
    type = token_type::semi_colon;
    break;
  case ',':
    type = token_type::comma;
    break;
  case '{':
    type = token_type::left_brace;
    break;
  case '}':
    type = token_type::right_brace;
    break;
  case '[':
    type = token_type::left_bracket;
    break;
  case ']':
    type = token_type::right_bracket;
    break;

  default:
    return _fail(diag_code::unexpected_character, begin, begin + 1);
  }

  _advance();
  return _make(type, begin);
}

lexer::lex_result lexer::_lex_ident() {
  const auto begin = _pos;
  _advance_while(is_ident_continue);
  return _make(token_type::identifier, begin);
}

lexer::lex_result lexer::_lex_string() {
  const auto begin = _pos;
  const char quote = _peek();
  _advance();

  while (!_at_end()) {
    const char c = _peek();

    if (c == quote) {
      _advance();
      return _make(token_type::string_literal, begin);
    }

    if (c == '\n')
      return _fail(diag_code::newline_in_string, begin, _pos);

    if (c == '\\') {
      const auto esc = _pos;
      _advance();

      if (!is_escape(_peek()))
        return _fail(diag_code::invalid_escape, esc,
                     std::min(_pos + 1, _source.size()));
    }

    _advance();
  }

  return _fail(diag_code::unterminated_string, begin, _pos);
}

lexer::lex_result lexer::_lex_number() {
  const auto begin = _pos;
  bool is_float = false;

  const auto malformed = [&] {
    _advance_while(is_ident_continue);
    return _fail(diag_code::malformed_number, begin, _pos);
  };

  if (is_sign(_peek()))
    _advance();
  _advance_while(is_digit);

  if (_peek() == '.' && is_digit(_peek(1))) {
    is_float = true;
    _advance();
    _advance_while(is_digit);
  }

  if (_peek() == 'e' || _peek() == 'E') {
    std::size_t i = 1;
    if (is_sign(_peek(i)))
      ++i;

    if (!is_digit(_peek(i)))
      return malformed();

    is_float = true;
    _advance(i);
    _advance_while(is_digit);
  }

  if (is_ident_continue(_peek()))
    return malformed();

  return _make(is_float ? token_type::float_literal
                        : token_type::integer_literal,
               begin);
}

} // namespace orchard::dot
