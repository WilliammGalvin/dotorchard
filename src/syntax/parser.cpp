#include "orchard/dot/syntax/token_type.hpp"
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <orchard/dot/base/diagnostic.hpp>
#include <orchard/dot/syntax/parser.hpp>
#include <span>
#include <string>

namespace orchard::dot {

namespace {

class parser {
public:
  explicit parser(std::span<const token> tokens) : _tokens(tokens) {}

  std::expected<value, diagnostic> parse_file() {
    table root;
    if (auto res = _parse_items(root, nullptr); !res)
      return std::unexpected(std::move(res.error()));

    const auto span = _join(_tokens.front().span, _tokens.back().span);
    return value(std::move(root)).with_span(span);
  }

private:
  using vresult = std::expected<value, diagnostic>;
  using status = std::expected<void, diagnostic>;

  std::span<const token> _tokens;
  std::size_t _pos{0};
  int _depth{0};

  class depth_guard {
  public:
    explicit depth_guard(parser &p) : _parser(p) { ++_parser._depth; }
    ~depth_guard() { --_parser._depth; }

  private:
    parser &_parser;
  };

  const token &_curr() const { return _tokens[_pos]; }

  const token &_prev() const { return _tokens[_pos - 1]; }

  bool _at(token_type type) const { return _curr().type == type; }

  const token &_take() {
    const auto &tok = _curr();
    if (tok.type != token_type::eof)
      ++_pos;

    return tok;
  }

  static source_span _join(source_span a, source_span b) {
    return {a.file, a.begin, b.end};
  }

  static std::string _describe(const token &tok) {
    if (tok.type == token_type::eof)
      return "end of file";

    return "'" + std::string(tok.content) + "'";
  }

  std::unexpected<diagnostic> _fail_here(diag_code code,
                                         const std::string &expected) const {
    return std::unexpected(
        make_error(code, _curr().span,
                   "expected " + expected + ", found " + _describe(_curr())));
  }

  std::unexpected<diagnostic> _fail_after_prev(diag_code code,
                                               const std::string &msg) const {
    const auto end = _prev().span.end;
    return std::unexpected(
        make_error(code, {_prev().span.file, end, end}, msg));
  }

  std::expected<void, diagnostic> _check_depth() const {
    if (_depth > max_nesting_depth)
      return std::unexpected(make_error(
          diag_code::nesting_too_deep, _curr().span,
          "nesting deeper than " + std::to_string(max_nesting_depth)));

    return {};
  }

  status _parse_items(table &out, const token *open) {
    while (true) {
      if (open) {
        if (_at(token_type::right_brace))
          return {};

        if (_at(token_type::eof))
          return std::unexpected(
              make_error(diag_code::unexpected_token, _curr().span,
                         "expected '}', found end of file")
                  .with_note(open->span, "unclosed '{' opened here"));
      } else if (_at(token_type::eof)) {
        return {};
      }

      if (auto res = _parse_item(out); !res)
        return res;
    }
  }

  status _parse_item(table &out) {
    if (!_at(token_type::identifier))
      return _fail_here(diag_code::unexpected_token, "a key");

    const token key_token = _take();
    std::string key{key_token.content};

    vresult val = [&]() -> vresult {
      if (_at(token_type::equals)) {
        _take();

        auto v = _parse_value();
        if (!v)
          return v;

        if (!_at(token_type::semi_colon))
          return _fail_after_prev(diag_code::expected_semicolon,
                                  "expected ';' after value");

        _take();
        return v;
      }

      if (_at(token_type::left_brace))
        return _parse_table();

      return _fail_here(diag_code::unexpected_token, "'=' or '{' after key");
    }();

    if (!val)
      return std::unexpected(std::move(val.error()));

    auto [where, inserted] =
        out.insert(entry{std::move(key), std::move(*val), key_token.span});
    if (!inserted)
      return std::unexpected(
          make_error(diag_code::duplicate_key, key_token.span,
                     "duplicate key '" + std::string(key_token.content) + "'")
              .with_note(where->key_span, "first defined here"));

    return {};
  }

  vresult _parse_value() {
    switch (_curr().type) {
    case token_type::string_literal:
      return _parse_string();

    case token_type::integer_literal:
      return _parse_integer();

    case token_type::float_literal:
      return _parse_float();

    case token_type::left_bracket:
      return _parse_list();

    case token_type::left_brace:
      return _parse_table();

    case token_type::identifier: {
      const auto &token = _curr();
      if (token.content == "true" || token.content == "false") {
        _take();
        return value(token.content == "true").with_span(token.span);
      }

      return _fail_here(diag_code::expected_value, "a value");
    }

    default:
      return _fail_here(diag_code::expected_value, "a value");
    }
  }

  vresult _parse_table() {
    const token open = _take();
    depth_guard guard(*this);
    if (auto depth = _check_depth(); !depth)
      return std::unexpected(std::move(depth.error()));

    table t;
    if (auto res = _parse_items(t, &open); !res)
      return std::unexpected(std::move(res.error()));

    const token close = _take();
    return value(std::move(t)).with_span(_join(open.span, close.span));
  }

  vresult _parse_list() {
    const token open = _take();
    depth_guard guard(*this);
    if (auto depth = _check_depth(); !depth)
      return std::unexpected(std::move(depth.error()));

    list items;
    while (!_at(token_type::right_bracket)) {
      if (_at(token_type::eof))
        return std::unexpected(
            make_error(diag_code::unexpected_token, _curr().span,
                       "expected ']', found end of file")
                .with_note(open.span, "unclosed '[' opened here"));

      auto v = _parse_value();
      if (!v)
        return v;
      items.push_back(std::move(*v));

      if (_at(token_type::comma)) {
        _take();
        continue;
      }

      if (!_at(token_type::right_bracket))
        return _fail_here(diag_code::unexpected_token, "',' or ']'");
    }

    const token close = _take();
    return value(std::move(items)).with_span(_join(open.span, close.span));
  }

  vresult _parse_string() {
    const token tok = _take();
    const auto body = tok.content.substr(1, tok.content.size() - 2);

    std::string out;
    out.reserve(body.size());
    for (std::size_t i = 0; i < body.size(); ++i) {
      if (body[i] != '\\') {
        out += body[i];
        continue;
      }

      switch (body[++i]) {
      case 'n':
        out += '\n';
        break;
      case 't':
        out += '\t';
        break;
      case 'r':
        out += '\r';
        break;
      case '0':
        out += '\0';
        break;

      default:
        out += body[i];
        break;
      }
    }

    return value(std::move(out)).with_span(tok.span);
  }

  vresult _parse_integer() {
    const token tok = _take();
    auto text = tok.content;
    if (!text.empty() && text.front() == '+')
      text.remove_prefix(1);

    std::int64_t v{};
    const auto [ptr, ec] =
        std::from_chars(text.data(), text.data() + text.size(), v);
    if (ec == std::errc::result_out_of_range)
      return std::unexpected(make_error(diag_code::integer_overflow, tok.span));

    if (ec != std::errc{} || ptr != text.data() + text.size())
      return std::unexpected(make_error(diag_code::malformed_number, tok.span));

    return value(v).with_span(tok.span);
  }

  vresult _parse_float() {
    const token tok = _take();
    auto text = tok.content;
    if (!text.empty() && text.front() == '+')
      text.remove_prefix(1);

    double v{};
    const auto [ptr, ec] =
        std::from_chars(text.data(), text.data() + text.size(), v);
    if (ec == std::errc::result_out_of_range)
      return std::unexpected(
          make_error(diag_code::float_out_of_range, tok.span));

    if (ec != std::errc{} || ptr != text.data() + text.size())
      return std::unexpected(make_error(diag_code::malformed_number, tok.span));

    return value(v).with_span(tok.span);
  }
};

} // namespace

std::expected<value, diagnostic> parse(std::span<const token> tokens) {
  return parser(tokens).parse_file();
}

} // namespace orchard::dot
