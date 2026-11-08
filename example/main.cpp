#include <iostream>
#include <orchard/dot.hpp>
#include <string>

using namespace orchard::dot;

namespace {

int failures = 0;

void check(bool ok, const char *what) {
  std::cout << (ok ? "  [ok]   " : "  [FAIL] ") << what << '\n';
  if (!ok)
    ++failures;
}

void section(const char *title) { std::cout << "\n== " << title << " ==\n"; }

// Parses `text` and expects a specific error code.
void expect_error(const char *label, std::string text, diag_code code) {
  source_map map;
  auto doc = parse_string(map, "<test>", std::move(text));
  if (doc) {
    check(false, label);
    return;
  }
  const bool match = doc.error().code == code;
  check(match, label);
  std::cout << render(doc.error(), map);
}

} // namespace

int main(int argc, char **argv) {
  // ---------------------------------------------------------------- file
  section("load file");
#ifdef ORCHARD_EXAMPLE_FILE
  const char *path = argc > 1 ? argv[1] : ORCHARD_EXAMPLE_FILE;
#else
  if (argc < 2) {
    std::cerr << "usage: example <file.orchard>\n";
    return 2;
  }
  const char *path = argv[1];
#endif

  source_map map;
  auto doc = load_path(map, path);
  if (!doc) {
    std::cerr << render(doc.error(), map);
    return 1;
  }

  const table &root = *doc->as_table();

  if (const auto *v = root.get("name"))
    if (const auto *s = v->as_string())
      std::cout << "  name      = " << *s << '\n';
  if (const auto *v = root.get("opt_level"))
    if (const auto *n = v->as_int())
      std::cout << "  opt_level = " << *n << '\n';
  if (const auto *v = root.get("features"))
    if (const auto *l = v->as_list()) {
      std::cout << "  features  = (" << l->size() << " items)";
      for (const auto &item : *l)
        if (const auto *s = item.as_string())
          std::cout << ' ' << *s;
      std::cout << '\n';
      check(l->size() == 2, "features has 2 items");
    }
  std::cout << "  keys:";
  for (const auto &e : root)
    std::cout << ' ' << e.key;
  std::cout << '\n';

  // -------------------------------------------------------- from a string
  section("parse string");
  {
    source_map m;
    auto d = parse_string(m, "<inline>", R"(
      a = 1;
      b { c = "x"; d = [1, 2, 3,]; }
      e = { f = true; };
      g = [];
      h = -2.5e3;
    )");
    check(d.has_value(), "parses");
    if (d) {
      const table &t = *d->as_table();
      check(t.get("a") && *t.get("a")->as_int() == 1, "a == 1");
      check(t.get("b")->as_table()->get("c") &&
                *t.get("b")->as_table()->get("c")->as_string() == "x",
            "b.c == \"x\"");
      check(t.get("b")->as_table()->get("d")->as_list()->size() == 3,
            "b.d has 3 items (trailing comma ok)");
      check(*t.get("e")->as_table()->get("f")->as_bool(), "e.f == true");
      check(t.get("g")->as_list()->empty(), "g is empty list");
      check(*t.get("h")->as_float() == -2500.0, "h == -2500.0");
    } else {
      std::cout << render(d.error(), m);
    }
  }

  // -------------------------------------------------- block == '=' form
  section("block sugar");
  {
    source_map m1, m2;
    auto a = parse_string(m1, "<a>", "deps { x = 1; }");
    auto b = parse_string(m2, "<b>", "deps = { x = 1; };");
    check(a && b && *a == *b, "key { } equals key = { };");
  }

  // ------------------------------------------------------ int vs float
  section("1 vs 1.0");
  {
    source_map m;
    auto d = parse_string(m, "<t>", "a = 1; b = 1.0;");
    check(d && d->as_table()->get("a")->kind() == value_type::integer,
          "1 is an integer");
    check(d && d->as_table()->get("b")->kind() == value_type::floating,
          "1.0 is a float");
  }

  // ----------------------------------------------------- build in code
  section("build + write");
  value cfg =
      table{}
          .set("name", "cherry")
          .set("opt_level", 2)
          .set("ratio", 1.0)
          .set("quote", "say \"hi\"\n")
          .set("features", list{"simd", "lto"})
          .set("target", table{}
                             .set("cpu", "znver4")
                             .set("flags", list{"-O2", "-fno-exceptions"}))
          .set("servers",
               list{table{}.set("host", "a"), table{}.set("host", "b")})
          .set("empty", table{});

  auto text = write(cfg);
  if (!text) {
    std::cerr << text.error().message << '\n';
    return 1;
  }
  std::cout << *text;

  // -------------------------------------------------------- round trip
  section("round trip");
  {
    source_map m;
    auto again = parse_string(m, "<roundtrip>", *text);
    if (!again) {
      std::cout << render(again.error(), m);
      check(false, "written text re-parses");
    } else {
      check(*again == cfg, "parse(write(v)) == v (built tree)");
    }
  }
  {
    // the file we loaded, written back out and re-parsed
    auto t2 = write(*doc);
    check(t2.has_value(), "loaded file can be written");
    if (t2) {
      source_map m;
      auto again = parse_string(m, "<roundtrip-file>", *t2);
      check(again && *again == *doc, "parse(write(file)) == file");
    }
  }

  // ------------------------------------------------------- writer errors
  section("writer errors");
  {
    auto r = write(value(table{}.set("bad key", 1)));
    check(!r && r.error().code == diag_code::invalid_key, "invalid_key");

    auto r2 =
        write(value(table{}.set("x", std::numeric_limits<double>::infinity())));
    check(!r2 && r2.error().code == diag_code::non_finite_float,
          "non_finite_float");

    auto r3 = write(value(5));
    check(!r3 && r3.error().code == diag_code::root_not_table,
          "root_not_table");
  }

  // ------------------------------------------------------- parse errors
  section("parse errors");
  expect_error("expected_semicolon", "a = 1\nb = 2;",
               diag_code::expected_semicolon);
  expect_error("duplicate_key", "a = 1;\na = 2;", diag_code::duplicate_key);
  expect_error("expected_value", "a = ;", diag_code::expected_value);
  expect_error("unexpected_token (no '=')", "a 1;",
               diag_code::unexpected_token);
  expect_error("unclosed brace", "a { b = 1;", diag_code::unexpected_token);
  expect_error("unclosed bracket", "a = [1, 2;", diag_code::unexpected_token);
  expect_error("unterminated_string", "a = \"abc",
               diag_code::unterminated_string);
  expect_error("newline_in_string", "a = \"ab\ncd\";",
               diag_code::newline_in_string);
  expect_error("invalid_escape", "a = \"\\q\";", diag_code::invalid_escape);
  expect_error("malformed_number", "a = 12abc;", diag_code::malformed_number);
  expect_error("integer_overflow", "a = 9223372036854775808;",
               diag_code::integer_overflow);
  expect_error("unexpected_character", "a = @;",
               diag_code::unexpected_character);
  expect_error("stray '}'", "}", diag_code::unexpected_token);
  expect_error("nesting_too_deep",
               "a = " + std::string(200, '[') + std::string(200, ']') + ";",
               diag_code::nesting_too_deep);

  // ------------------------------------------------------------ io errors
  section("io errors");
  {
    source_map m;
    auto r = load_path(m, "does_not_exist.orchard");
    check(!r && r.error().code == diag_code::io_failure, "missing file");
    if (!r)
      std::cout << render(r.error(), m);

    auto r2 = load_path(m, "notes.txt");
    check(!r2 && r2.error().code == diag_code::io_failure, "wrong extension");
    if (!r2)
      std::cout << render(r2.error(), m);
  }

  std::cout << "\n"
            << (failures == 0 ? "ALL PASSED" : "FAILURES: ")
            << (failures == 0 ? "" : std::to_string(failures)) << '\n';
  return failures == 0 ? 0 : 1;
}
