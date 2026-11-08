#pragma once

#include <expected>
#include <orchard/dot/base/diagnostic.hpp>
#include <orchard/dot/base/file_io.hpp>
#include <orchard/dot/base/source_file.hpp>
#include <orchard/dot/model/value.hpp>
#include <string>

namespace orchard::dot {

[[nodiscard]] std::expected<value, diagnostic>
parse_source(const source_map &map, file_id id);

[[nodiscard]] std::expected<value, diagnostic>
parse_string(source_map &map, std::string name, std::string text);

[[nodiscard]] std::expected<value, diagnostic> load_path(source_map &map,
                                                         const file_path &path);

} // namespace orchard::dot
