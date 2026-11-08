#pragma once

#include <cstddef>
#include <expected>
#include <orchard/dot/base/diagnostic.hpp>
#include <orchard/dot/model/value.hpp>
#include <string>

namespace orchard::dot {

struct write_options {
  int indent{2};
  std::size_t max_inline_width{80};
};

[[nodiscard]] std::expected<std::string, diagnostic>
write(const value &root, const write_options &opts = {});

} // namespace orchard::dot
