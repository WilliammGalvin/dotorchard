#include <fstream>
#include <ios>
#include <limits>
#include <new>
#include <orchard/dot/base/file_io.hpp>
#include <system_error>

namespace orchard::dot {

std::expected<std::string, io_error> read_text_file(const file_path &path) {
  namespace fs = std::filesystem;

  std::error_code ec;
  const auto st = fs::status(path, ec);
  if (st.type() == fs::file_type::not_found)
    return std::unexpected(io_error::not_found);

  if (ec || !fs::is_regular_file(st))
    return std::unexpected(io_error::failed_to_open);

  const auto size = fs::file_size(path, ec);
  if (ec)
    return std::unexpected(io_error::failed_to_open);

  if (size > std::numeric_limits<std::uint32_t>::max())
    return std::unexpected(io_error::too_large);

  std::ifstream in(path, std::ios::binary);
  if (!in)
    return std::unexpected(io_error::failed_to_open);

  std::string text;
  try {
    text.resize(static_cast<std::size_t>(size));
  } catch (const std::bad_alloc &) {
    return std::unexpected(io_error::too_large);
  }

  const auto bytes = static_cast<std::streamsize>(size);
  in.read(text.data(), bytes);
  if (in.gcount() != bytes) // also catches a file that shrank after file_size
    return std::unexpected(io_error::failed_to_read);

  return text;
}

} // namespace orchard::dot
