#include <iostream>
#include <orchard/dot.hpp>

int main(int argc, char **argv) {
  using namespace orchard::dot;

  if (argc < 2) {
    std::cerr << "usage: example <file.orchard>\n";
    return 2;
  }

  source_map map;

  auto doc = load_path(map, argv[1]);
  if (!doc) {
    std::cerr << render(doc.error(), map);
    return 1;
  }

  const table &root = *doc->as_table();

  if (const auto *v = root.get("name"))
    if (const auto *s = v->as_string())
      std::cout << "name      = " << *s << '\n';

  if (const auto *v = root.get("opt_level"))
    if (const auto *n = v->as_int())
      std::cout << "opt_level = " << *n << '\n';

  if (const auto *v = root.get("pure"))
    if (const auto *b = v->as_bool())
      std::cout << "pure      = " << std::boolalpha << *b << '\n';

  if (const auto *v = root.get("features"))
    if (const auto *l = v->as_list()) {
      std::cout << "features  =";
      for (const auto &item : *l)
        if (const auto *s = item.as_string())
          std::cout << ' ' << *s;
      std::cout << '\n';
    }

  if (const auto *t = root.get("target"))
    if (const auto *tt = t->as_table())
      if (const auto *cpu = tt->get("cpu"))
        if (const auto *s = cpu->as_string())
          std::cout << "target.cpu = " << *s << '\n';

  std::cout << "\nkeys in order:";
  for (const auto &e : root)
    std::cout << ' ' << e.key;
  std::cout << '\n';
}
