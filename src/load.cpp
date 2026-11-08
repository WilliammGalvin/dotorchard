#include <orchard/dot/load.hpp>
#include <orchard/dot/syntax/lexer.hpp>
#include <orchard/dot/syntax/parser.hpp>

namespace orchard::dot {

std::expected<value, diagnostic> parse_source(const source_map &map,
                                              file_id id) {
  const auto &file = map.get(id);
  auto tokens = lexer(file.text(), id).lex();
  if (!tokens)
    return std::unexpected(std::move(tokens.error()));

  return parse(*tokens);
}

std::expected<value, diagnostic> parse_string(source_map &map, std::string name,
                                              std::string text) {
  auto id = map.add(name, std::move(text));
  if (!id)
    return std::unexpected(make_io_error(name, id.error()));

  return parse_source(map, *id);
}

std::expected<value, diagnostic> load_path(source_map &map,
                                           const file_path &path) {
  if (path.extension() != ".orchard")
    return std::unexpected(make_io_error(path, io_error::not_orchard_file));

  auto id = map.add_file(path);
  if (!id)
    return std::unexpected(make_io_error(path, id.error()));

  return parse_source(map, *id);
}

} // namespace orchard::dot
