# dot.orchard

`dot.orchard` is a human-readable file format for storing data,
in the same space as TOML and JSON. Files use the `.orchard` extension.

`.orchard` files are the format behind the manifest and configuration
files in many of my upcoming portfolio projects, including the orchard build system
and the petal library.

## Overview

This project is a C++ 23 library for creating, parsing, and writing
`.orchard` files efficiently and conveniently.

Everything is hand-rolled, with no third-party dependencies. There are
a few central pieces of this project:

**Lexer**
Turns the raw document text into a stream of tokens.

**Parser**
Builds an in-memory document (tree of values) from those tokens.

**Writer**
Serializes a document back out as an `.orchard` file.

**Verifier**
Checks that a document has the structure it is supposed
to have, using a schema written as an `.orchard` file.

## Format

Orchard files are made of key-value pairs:

```JSON
# project config
name = "cherry";
version = 1;
opt_level = 2;
pure = true;
ratio = 0.75;
features = ["foo", "bar"];

target {
  cpu = "amd";
  flags = ["-O2", "-fno-exceptions"];
}

deps {
  petal = {
    path = "../petal";
  };
}
```

### Rules

- A pair is a `key = value;`. The semicolon is required,
the grammar doesn't require any specified indentation or new lines.
- `key { ... }` opens a section. It is a shorthand for `key = { ... };`,
with no `=` and no trailing `;`;
- Keys are identifiers: letters, digits and `_`, not starting with a digit.
- Duplicate keys are an error. Key order is **preserved**.
- `#` starts a comment that runs until the end of the line.

### Types

**Primitive Types**:

- **String**: `"cherry"`, `"say \"hi\"\n"`
- **Integer**: `42`, `-7`, `+3`
- **Float**: `0.75`, `-2.5e3`
- **Boolean**: `true`, `false`
- **List**: `[1, 2, 3]`, `[]`
- **Table**: `{ a = 1; }`

**Notes**:

- `1` and `1.0` are different types
- Integers are 64-bit signed
- Lists may have a trailing comma
- Strings support escape characters

## Library Usage

### Parse a file

```cpp
#include <orchard/dot.hpp>

using namespace orchard::dot;

source_map map;
auto doc = load_path(map, "cherry.orchard");
if (!doc) {
  std::cerr << render(doc.error(), map);
  return 1;
}

const table& root = *doc->as_table();
if (const auto* name = root.get("name"))
  std::cout << *name->as_string() << '\n';
```

### Load into your own structs

```cpp
#include <orchard/dot.hpp>

using namespace orchard::dot;

...

struct target_cfg {
  std::string cpu;
  std::vector<std::string> flags;
};

convert_result<target_cfg> from_value(tag<target_cfg>, const value &v) {
  table_reader reader(v);
  target_cfg cfg;
  reader.required("cpu", cfg.cpu).optional("flags", cfg.flags);
  
  return reader.finish(std::move(cfg));
}

auto cfg = load_as<target_cfg>(map, "target.orchard");
```

### Write a document

```cpp
#include <orchard/dot.hpp>

using namespace orchard::dot;

...

value cfg = table{}
  .set("name", "cherry")
  .set("features", list{"bar", "foo"});

auto text = write(cfg);
```

### Validate with a schema

```JSON
fields {
  name {
    type = "string";
    required = true;
  }
  opt_level {
    type = "int";
    min = 0;
    max = 3;
    default = 2;
  }
  features {
    type = "list";
    of = "string";
  }
}
```

```cpp
#include <orchard/dot.hpp>

using namespace orchard::dot;

...

auto schema = load_schema(map, "cherry.schema.orchard");
apply_defaults(*doc, *schema);
auto errors = validate(*doc, *schema);
```

## Building project

From root directory:

```sh
cmake -B build
cmake --build build
./build/example/example example/cherry.orchard
```

To use it from another CMake project:

```cmake
add_subdirectory(dotorchard)
target_link_libraries(my_target PRIVATE orchard::dot)
```
