#include <algorithm>
#include <limits>
#include <orchard/dot/base/source_file.hpp>
#include <stdexcept>
#include <utility>

namespace orchard::dot {

namespace {

constexpr std::size_t max_text_size = std::numeric_limits<std::uint32_t>::max();

} // namespace

source_file::source_file(std::string name, std::string text)
    : _name(std::move(name)), _text(std::move(text)) {
  if (_text.size() > max_text_size)
    throw std::length_error("source_file: text exceeds 4 GiB");

  _line_starts.push_back(0);
  for (std::size_t i = 0; i < _text.size(); ++i)
    if (_text[i] == '\n')
      _line_starts.push_back(static_cast<std::uint32_t>(i + 1));
}

line_col source_file::locate(std::uint32_t offset) const noexcept {
  offset = std::min(offset, static_cast<std::uint32_t>(_text.size()));

  const auto it =
      std::upper_bound(_line_starts.begin(), _line_starts.end(), offset);
  const auto idx = static_cast<std::uint32_t>(it - _line_starts.begin()) - 1;
  return {idx + 1, offset - _line_starts[idx] + 1};
}

std::string_view source_file::line_text(std::uint32_t line) const noexcept {
  if (line == 0 || line > _line_starts.size())
    return {};

  const auto begin = _line_starts[line - 1];
  auto end = line < _line_starts.size()
                 ? _line_starts[line] - 1 // drop '\n'
                 : static_cast<std::uint32_t>(_text.size());

  if (end > begin && _text[end - 1] == '\r')
    --end;

  return std::string_view(_text).substr(begin, end - begin);
}

std::string_view source_file::slice(std::uint32_t begin,
                                    std::uint32_t end) const noexcept {
  const auto n = static_cast<std::uint32_t>(_text.size());
  begin = std::min(begin, n);
  end = std::clamp(end, begin, n);
  return std::string_view(_text).substr(begin, end - begin);
}

std::expected<file_id, io_error> source_map::add(std::string name,
                                                 std::string text) {
  if (text.size() > max_text_size || _files.size() >= no_file)
    return std::unexpected(io_error::too_large);

  _files.emplace_back(std::move(name), std::move(text));
  return static_cast<file_id>(_files.size() - 1);
}

std::expected<file_id, io_error> source_map::add_file(const file_path &path) {
  auto text = read_text_file(path);
  if (!text)
    return std::unexpected(text.error());

  return add(path.string(), std::move(*text));
}

} // namespace orchard::dot
