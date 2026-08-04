# Agent Rules

## C++ Includes Rule
- When writing C++ code or including new libraries/headers for this project, **always** place the `#include <...>` directives in `src/global.h`, unless specifically instructed otherwise.
- **Do not** add `#include` directives to individual `.cpp` or `.h` files directly (e.g., `#include <QDebug>`, `#include <QLabel>`, etc. must go in `global.h`).

## Architecture Guide Update Rule
- After you change the project's architecture, add new modules, or make any significant structural changes, you **must** update the `ARCHITECTURE.md` file in the root directory to reflect those changes. Keep the guide in sync with the codebase.

## No Unnecessary Code Change Rule
- **Do not** change, add, or remove code in the project unless it is directly necessary to fulfill the user's request or fix a bug.
- **Avoid** refactoring, rearranging, or "cleaning up" code (e.g., changing variable names, reordering methods, adding comments) if it does not directly contribute to solving the task at hand.
- **Respect** the existing codebase structure and style. Only modify files that are relevant to the current objective.

## MVC Architecture Rule
- **Strictly adhere to the Model-View-Controller (MVC) design pattern** when adding or modifying features.
- **Model (`src/model/`)**: Must encapsulate all data logic, state, and database interactions. Models should never include UI-specific headers or know about Views.
- **View (`src/view/`)**: Must strictly handle UI components and presentation. Views should not contain business logic or direct database queries; they should delegate actions to Controllers.
- **Controller (`src/control/`)**: Must act as the intermediary, orchestrating data flow between Models and Views. Controllers handle user input from Views and update Models accordingly.
- **MVC Best Practices**:
  - **Signals and Slots**: Use Qt's signal and slot mechanism to communicate between layers loosely (e.g., View emits a signal to Controller on user action, Model emits a signal to Controller on data change).
  - **Header and Dependency Isolation**: `src/model/` must NEVER include UI headers (`<QWidget>`, `<QPushButton>`, etc.). `src/view/` must NEVER include database headers (`<QSqlDatabase>`, etc.).
  - **Dependency Injection**: Instantiate Models centrally and pass pointers to Controllers. Controllers can then construct Views.
  - **DTOs**: Use simple structs (Data Transfer Objects) to pass data between layers rather than heavy Model objects.

## No Unicode Emojis/Icons in Code Rule
- **Avoid** using Unicode emojis or text-based icons directly in the source code or UI strings (e.g., 😀, 👍, etc.).
- If an icon is needed in the UI, **always** use an external SVG file and load it via Qt's resource system or file paths.
