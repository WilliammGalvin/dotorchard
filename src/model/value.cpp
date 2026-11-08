#include <algorithm>
#include <orchard/dot/model/value.hpp>

namespace orchard::dot {

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

} // namespace orchard::dot
