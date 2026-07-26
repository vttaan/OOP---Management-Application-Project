# Agent Rules

## C++ Includes Rule
- When writing C++ code or including new libraries/headers for this project, **always** place the `#include <...>` directives in `src/global.h`, unless specifically instructed otherwise.
- **Do not** add `#include` directives to individual `.cpp` or `.h` files directly (e.g., `#include <QDebug>`, `#include <QLabel>`, etc. must go in `global.h`).
