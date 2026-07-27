# Agent Rules

## C++ Includes Rule
- When writing C++ code or including new libraries/headers for this project, **always** place the `#include <...>` directives in `src/global.h`, unless specifically instructed otherwise.
- **Do not** add `#include` directives to individual `.cpp` or `.h` files directly (e.g., `#include <QDebug>`, `#include <QLabel>`, etc. must go in `global.h`).

## Architecture Guide Update Rule
- After you change the project's architecture, add new modules, or make any significant structural changes, you **must** update the `ARCHITECTURE.md` file in the root directory to reflect those changes. Keep the guide in sync with the codebase.
