#pragma once

#include <cstdint>

namespace orchard::dot {

using file_id = std::uint32_t;
inline constexpr file_id no_file = ~file_id{0};

struct source_span {
  file_id file{no_file};
  std::uint32_t begin{0};
  std::uint32_t end{0};

  [[nodiscard]] constexpr bool has_file() const noexcept {
    return file != no_file;
  }

  [[nodiscard]] constexpr std::uint32_t size() const noexcept {
    return end - begin;
  }

  friend constexpr bool operator==(const source_span &,
                                   const source_span &) = default;
};

inline constexpr source_span no_span{};

} // namespace orchard::dot
