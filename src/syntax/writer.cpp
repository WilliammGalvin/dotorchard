#include <algorithm>
#include <cmath>
#include <cstddef>
#include <expected>
#include <orchard/dot/base/diagnostic.hpp>
#include <orchard/dot/model/value.hpp>
#include <orchard/dot/syntax/parser.hpp>
#include <orchard/dot/syntax/writer.hpp>
#include <string>
#include <string_view>

namespace orchard::dot {

namespace {

using status = std::expected<void, diagnostic>;

constexpr bool is_alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

constexpr bool is_digit(char c) { return c >= '0' && c <= '9'; }

bool is_identifier(std::string_view s) {
  if (s.empty() || !(is_alpha(s[0]) || s[0] == '_'))
    return false;

  for (char c : s) {
    if (!(is_alpha(c) || is_digit(c) || c == '_'))
      return false;
  }

  return true;
}

bool is_scalar(const value &v) {
  return v.kind() != value_type::list && v.kind() != value_type::table;
}

class writer {
public:
  explicit writer(const write_options &opts) : _opts(opts) {}

  std::expected<std::string, diagnostic> run(const value &root) {
    const auto *tbl = root.as_table();
    if (!tbl)
      return std::unexpected(
          make_error(diag_code::root_not_table, root.span()));

    if (auto res = _items(*tbl, 0); !res)
      return std::unexpected(std::move(res.error()));

    return std::move(_out);
  }

private:
  const write_options &_opts;
  std::string _out;

  void _pad(int level) {
    _out.append(static_cast<std::size_t>(level) *
                    static_cast<std::size_t>(_opts.indent),
                ' ');
  }

  std::size_t _column(int level) const {
    return static_cast<std::size_t>(level) *
           static_cast<std::size_t>(_opts.indent);
  }

  static status _too_deep(const value &v) {
    return std::unexpected(
        make_error(diag_code::nesting_too_deep, v.span(),
                   "nesting deeper than " + std::to_string(max_nesting_depth)));
  }

  static void _append_string(std::string &out, const std::string &s) {
    out += '"';
    for (char c : s) {
      switch (c) {
      case '\n':
        out += "\\n";
        break;
      case '\t':
        out += "\\t";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\0':
        out += "\\0";
        break;
      case '\\':
        out += "\\\\";
        break;
      case '"':
        out += "\\\"";
        break;

      default:
        out += c;
        break;
      }
    }
    out += '"';
  }

  static status _append_scalar(std::string &out, const value &v) {
    switch (v.kind()) {
    case value_type::boolean:
      out += *v.as_bool() ? "true" : "false";
      return {};

    case value_type::integer: {
      char buf[24];
      const auto res = std::to_chars(buf, buf + sizeof(buf), *v.as_int());
      out.append(buf, res.ptr);
      return {};
    }

    case value_type::floating: {
      const double d = *v.as_float();
      if (!std::isfinite(d))
        return std::unexpected(
            make_error(diag_code::non_finite_float, v.span()));

      char buf[40];
      const auto res = std::to_chars(buf, buf + sizeof(buf), d);
      const std::string_view s(buf, static_cast<std::size_t>(res.ptr - buf));
      out += s;

      if (s.find_first_of(".eE") == std::string_view::npos)
        out += ".0";

      return {};
    }

    case value_type::string:
      _append_string(out, *v.as_string());
      return {};

    default:
      return {};
    }
  }

  status _value_text(const value &v, int level, std::size_t prefix) {
    if (level > max_nesting_depth)
      return _too_deep(v);

    if (const auto *lst = v.as_list())
      return _list_text(*lst, v, level, prefix);

    if (const auto *tbl = v.as_table()) {
      if (tbl->empty()) {
        _out += "{ }";
        return {};
      }

      _out += "{\n";
      if (auto res = _items(*tbl, level + 1); !res)
        return res;

      _pad(level);
      _out += '}';
      return {};
    }

    return _append_scalar(_out, v);
  }

  status _list_text(const list &lst, const value &self, int level,
                    std::size_t prefix) {
    if (lst.empty()) {
      _out += "[]";
      return {};
    }

    if (std::ranges::all_of(lst, is_scalar)) {
      std::string line = "[";
      for (std::size_t i = 0; i < lst.size(); ++i) {
        if (i > 0)
          line += ", ";

        if (auto res = _append_scalar(line, lst[i]); !res)
          return res;
      }
      line += ']';

      if (prefix + line.size() + 1 <= _opts.max_inline_width) {
        _out += line;
        return {};
      }
    }

    if (level + 1 > max_nesting_depth)
      return _too_deep(self);

    _out += "[\n";
    for (const auto &item : lst) {
      _pad(level + 1);
      if (auto res = _value_text(item, level + 1, _column(level + 1)); !res)
        return res;

      _out += ",\n";
    }

    _pad(level);
    _out += ']';
    return {};
  }

  status _items(const table &tbl, int level) {
    for (const auto &entry : tbl) {
      if (!is_identifier(entry.key))
        return std::unexpected(make_error(
            diag_code::invalid_key,
            entry.key_span.has_file() ? entry.key_span : entry.val.span(),
            "key '" + entry.key + "' is not a valid indentifier"));

      _pad(level);
      _out += entry.key;

      if (entry.val.kind() == value_type::table) {
        _out += ' ';
        if (auto res = _value_text(entry.val, level, 0); !res)
          return res;
        _out += '\n';
      } else {
        _out += " = ";
        if (auto res = _value_text(entry.val, level,
                                   _column(level) + entry.key.size() + 3);
            !res)
          return res;
        _out += ";\n";
      }
    }

    return {};
  }
};

} // namespace

std::expected<std::string, diagnostic> write(const value &root,
                                             const write_options &opts) {
  return writer(opts).run(root);
}

} // namespace orchard::dot
