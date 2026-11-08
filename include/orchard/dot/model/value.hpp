#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <orchard/dot/base/span.hpp>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace orchard::dot {

class value;
struct entry;

using list = std::vector<value>;

enum class value_type : std::uint8_t {
  boolean,
  integer,
  floating,
  string,
  list,
  table,
};

[[nodiscard]] std::string_view type_name(value_type type) noexcept;

class table {
public:
  using container = std::vector<entry>;
  using iterator = container::iterator;
  using const_iterator = container::const_iterator;

  struct insert_result {
    entry *where;
    bool inserted;
  };

  [[nodiscard]] insert_result insert(entry &&e);

  table &set(std::string key, value v) &;
  table &&set(std::string key, value v) &&;

  [[nodiscard]] entry *find(std::string_view key) noexcept;

  [[nodiscard]] const entry *find(std::string_view key) const noexcept;

  [[nodiscard]] value *get(std::string_view key) noexcept;

  [[nodiscard]] const value *get(std::string_view key) const noexcept;

  [[nodiscard]] bool contains(std::string_view key) const noexcept {
    return find(key) != nullptr;
  }

  bool erase(std::string_view key);

  [[nodiscard]] std::size_t size() const noexcept;
  [[nodiscard]] bool empty() const noexcept;
  [[nodiscard]] iterator begin() noexcept;
  [[nodiscard]] iterator end() noexcept;
  [[nodiscard]] const_iterator begin() const noexcept;
  [[nodiscard]] const_iterator end() const noexcept;

private:
  container _entries;
};

[[nodiscard]] bool operator==(const table &a, const table &b);

class value {
public:
  using storage =
      std::variant<bool, std::int64_t, double, std::string, list, table>;

  value() : _data(std::in_place_type<table>) {}

  value(bool v) noexcept : _data(v) {}

  template <std::integral I>
    requires(!std::same_as<I, bool>)
  value(I v) noexcept : _data(static_cast<std::int64_t>(v)) {}

  value(double v) noexcept : _data(v) {}

  value(std::string v) : _data(std::move(v)) {}

  value(std::string_view v) : _data(std::string(v)) {}

  value(const char *v) : _data(std::string(v)) {}

  value(list v) : _data(std::move(v)) {}

  value(table v) : _data(std::move(v)) {}

  [[nodiscard]] value_type kind() const noexcept {
    return static_cast<value_type>(_data.index());
  }

  [[nodiscard]] source_span span() const noexcept { return _span; }

  void set_span(source_span source) noexcept { _span = source; }

  [[nodiscard]] value with_span(source_span source) && {
    _span = source;
    return std::move(*this);
  }

  template <class T> [[nodiscard]] const T *get_if() const noexcept {
    return std::get_if<T>(&_data);
  }

  template <class T> [[nodiscard]] T *get_if() noexcept {
    return std::get_if<T>(&_data);
  }

  [[nodiscard]] const bool *as_bool() const noexcept { return get_if<bool>(); }

  [[nodiscard]] bool *as_bool() noexcept { return get_if<bool>(); }

  [[nodiscard]] const std::int64_t *as_int() const noexcept {
    return get_if<std::int64_t>();
  }

  [[nodiscard]] std::int64_t *as_int() noexcept {
    return get_if<std::int64_t>();
  }

  [[nodiscard]] const double *as_float() const noexcept {
    return get_if<double>();
  }

  [[nodiscard]] double *as_float() noexcept { return get_if<double>(); }

  [[nodiscard]] const std::string *as_string() const noexcept {
    return get_if<std::string>();
  }

  [[nodiscard]] std::string *as_string() noexcept {
    return get_if<std::string>();
  }

  [[nodiscard]] const list *as_list() const noexcept { return get_if<list>(); }

  [[nodiscard]] list *as_list() noexcept { return get_if<list>(); }

  [[nodiscard]] const table *as_table() const noexcept {
    return get_if<table>();
  }

  [[nodiscard]] table *as_table() noexcept { return get_if<table>(); }

  [[nodiscard]] const storage &data() const noexcept { return _data; }

  [[nodiscard]] storage &data() noexcept { return _data; }

  friend bool operator==(const value &a, const value &b) {
    return a._data == b._data;
  }

private:
  storage _data;
  source_span _span{};
};

struct entry {
  std::string key;
  value val;
  source_span key_span{};
};

[[nodiscard]] inline bool operator==(const entry &a, const entry &b) {
  return a.key == b.key && a.val == b.val;
}

inline std::size_t table::size() const noexcept { return _entries.size(); }

inline bool table::empty() const noexcept { return _entries.empty(); }

inline table::iterator table::begin() noexcept { return _entries.begin(); }

inline table::iterator table::end() noexcept { return _entries.end(); }

inline table::const_iterator table::begin() const noexcept {
  return _entries.begin();
}

inline table::const_iterator table::end() const noexcept {
  return _entries.end();
}

} // namespace orchard::dot
