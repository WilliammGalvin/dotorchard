#pragma once

#include <cstdint>
#include <deque>
#include <expected>
#include <orchard/dot/base/file_io.hpp>
#include <orchard/dot/base/span.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace orchard::dot {

struct line_col {
  std::uint32_t line;
  std::uint32_t column;
};

class source_file {
public:
  source_file(std::string name, std::string text);

  source_file(const source_file &) = delete;
  source_file &operator=(const source_file &) = delete;
  source_file(source_file &&) = delete;
  source_file &operator=(source_file &&) = delete;

  [[nodiscard]] std::string_view name() const noexcept { return _name; }
  [[nodiscard]] std::string_view text() const noexcept { return _text; }
  [[nodiscard]] std::size_t size() const noexcept { return _text.size(); }
  [[nodiscard]] std::size_t line_count() const noexcept {
    return _line_starts.size();
  }

  [[nodiscard]] line_col locate(std::uint32_t offset) const noexcept;

  [[nodiscard]] std::string_view line_text(std::uint32_t line) const noexcept;

  [[nodiscard]] std::string_view slice(std::uint32_t begin,
                                       std::uint32_t end) const noexcept;

  [[nodiscard]] std::string_view slice(source_span span) const noexcept {
    return slice(span.begin, span.end);
  }

private:
  std::string _name;
  std::string _text;
  std::vector<std::uint32_t> _line_starts;
};

class source_map {
public:
  [[nodiscard]] std::expected<file_id, io_error> add(std::string name,
                                                     std::string text);

  [[nodiscard]] std::expected<file_id, io_error>
  add_file(const file_path &path);

  [[nodiscard]] const source_file &get(file_id id) const {
    return _files.at(id);
  }

  [[nodiscard]] std::size_t size() const noexcept { return _files.size(); }

private:
  std::deque<source_file> _files;
};

} // namespace orchard::dot
