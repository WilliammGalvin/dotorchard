#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

namespace orchard::dot {

using file_path = std::filesystem::path;

enum class io_error : std::uint8_t {
  not_found,
  not_orchard_file,
  failed_to_open,
  failed_to_read,
  too_large,
};

constexpr std::string_view to_string(io_error error) noexcept {
  switch (error) {
  case io_error::not_found:
    return "file not found";
  case io_error::not_orchard_file:
    return "not an Orchard file";
  case io_error::failed_to_open:
    return "failed to open file";
  case io_error::failed_to_read:
    return "failed to read file";
  case io_error::too_large:
    return "file is too large";
  }

  return "unknown error";
}

[[nodiscard]] std::expected<std::string, io_error>
read_text_file(const file_path &path);

} // namespace orchard::dot
