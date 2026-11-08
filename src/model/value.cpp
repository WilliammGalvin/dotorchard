#include <algorithm>
#include <orchard/dot/model/value.hpp>

namespace orchard::dot {

std::string_view type_name(value_type type) noexcept {
  switch (type) {
  case value_type::boolean:
    return "boolean";
  case value_type::integer:
    return "integer";
  case value_type::floating:
    return "float";
  case value_type::string:
    return "string";
  case value_type::list:
    return "list";
  case value_type::table:
    return "table";
  }

  return "unknown";
}

entry *table::find(std::string_view key) noexcept {
  const auto it = std::ranges::find(_entries, key, &entry::key);
  return it == _entries.end() ? nullptr : &*it;
}

const entry *table::find(std::string_view key) const noexcept {
  const auto it = std::ranges::find(_entries, key, &entry::key);
  return it == _entries.end() ? nullptr : &*it;
}

value *table::get(std::string_view key) noexcept {
  auto *e = find(key);
  return e ? &e->val : nullptr;
}

const value *table::get(std::string_view key) const noexcept {
  const auto *e = find(key);
  return e ? &e->val : nullptr;
}

table::insert_result table::insert(entry &&e) {
  if (auto *existing = find(e.key))
    return {existing, false};

  _entries.push_back(std::move(e));
  return {&_entries.back(), true};
}

table &table::set(std::string key, value v) & {
  if (auto *existing = find(key)) {
    existing->val = std::move(v);
    return *this;
  }

  _entries.push_back(entry{std::move(key), std::move(v), no_span});
  return *this;
}

table &&table::set(std::string key, value v) && {
  set(std::move(key), std::move(v));
  return std::move(*this);
}

bool table::erase(std::string_view key) {
  const auto it = std::ranges::find(_entries, key, &entry::key);
  if (it == _entries.end())
    return false;

  _entries.erase(it);
  return true;
}

bool operator==(const table &a, const table &b) {
  return std::equal(a.begin(), a.end(), b.begin(), b.end());
}

} // namespace orchard::dot
