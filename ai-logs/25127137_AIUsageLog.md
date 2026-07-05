# AI Usage Log — Student ID: 25127137

## Session Overview

**Date:** 2026-07-02 to 2026-07-03  
**Task:** Implementation of Profile View, View/Control Navigator architectures, circular dependency resolution, and UI integration.

---

## Session 1 — Profile View Design Guidance (~20:36, July 2)

Guided the student in building a custom profile widget in Qt C++, outlining the structure for circular avatars, personal details (DOB, phone number, etc.), and custom QSS styles (using gradients, border-radius, and rounded card styling in `styles.qss`). The student manually implemented the skeleton files (`Profile_View.h`, `Profile_View.cpp`, `Profile_View.ui`) based on this guidance.

---

## Session 2 — Build & Dependency Fixes (~23:58, July 2)

### Changes Made

#### ###1 — Circular Dependency Resolution
- `src/view/Login_View.h` & `src/control/Login_Control.h`: Forward-declared each other to break circular dependency, moving `#include` statements to the `.cpp` source files.
- `src/view/Login_View.cpp`: Included `Login_Control.h` and guarded connections.
- `src/control/Login_Control.cpp`: Included `Login_View.h` and passed `this` to the login view constructor.

#### ###2 — Empty/Blank Navigator Fix
- `src/view/View_Navigator.cpp`: Reordered layout initialization and cleared default placeholder pages from `QStackedWidget` (`stackedWidget->clear()`) before adding custom pages to ensure the login page is visible on startup.

#### ###3 — Stylesheet Loading Fix
- `src/main.cpp`: Uncommented stylesheet loading block and configured it to use the correct `QApplication` instance (`a`).

### Files Modified
| File | Action |
|---|---|
| `src/view/Login_View.h` | Updated — added forward declaration for `Login_Control` |
| `src/view/Login_View.cpp` | Updated — included `Login_Control.h` and updated constructors |
| `src/control/Login_Control.h` | Updated — added forward declaration for `Login_View` |
| `src/control/Login_Control.cpp` | Updated — included `Login_View.h` and updated constructor parameters |
| `src/view/View_Navigator.cpp` | Updated — fixed connect syntax and cleared default placeholder pages |
| `src/main.cpp` | Updated — uncommented and corrected stylesheet loading block |

---

## Session 3 — Resizing and Stacked Widget Fixes (~03:48, July 3)

### Changes Made

#### ###1 — StackedWidget Resizing Issue
- `src/view/View_Navigator.ui` & `src/view/View_Navigator.cpp`: Wrapped the `stackedWidget` inside a layout (`QVBoxLayout`) to ensure that pages dynamically resize and expand with the main navigator window rather than being fixed size.

### Files Modified
| File | Action |
|---|---|
| `src/view/View_Navigator.ui` | Updated — wrapped stackedWidget in a QVBoxLayout |
| `src/view/View_Navigator.cpp` | Updated — added layout logic and adjusted size policy |

---

## Session 4 — Login View Controller Linkage (~05:31, July 3)

### Changes Made

#### ###1 — Duplicate View Resolution
- `src/view/View_Navigator.cpp`: Changed `View_Navigator` constructor to use the view instance managed and returned by `login_control->getView()` instead of creating a second redundant `Login_View` instance.
- `src/view/Login_View.cpp`: Modified the constructor to assign the `Login_Control*` controller inside the initializer list to ensure reference consistency.

### Files Modified
| File | Action |
|---|---|
| `src/view/View_Navigator.cpp` | Updated — switched to `login_control->getView()` |
| `src/view/Login_View.cpp` | Updated — initialized controller in initializer list |

---

## Session 5 — Navigator Integration & View-Controller Linking (~17:14, July 3)

### Changes Made

#### ###1 — Incomplete Type Compilation Fixes
- `src/control/Control_Navigator.cpp`: Included `View_Navigator.h` to resolve incomplete type error.
- `src/control/Profile_Control.cpp`: Included `Profile_View.h` to resolve incomplete type error.

#### ###2 — Profile_View Default Constructor Parameter
- `src/view/Profile_View.h`: Added default parent parameter to constructor (`QWidget *parent = nullptr`) so it can be instantiated without arguments in `View_Navigator.cpp`.

#### ###3 — View Association Interface (`setView`)
- `src/control/Login_Control.h/.cpp`, `src/control/Profile_Control.h/.cpp`, `src/control/Dashboard_Control.h/.cpp`: Added `setView` method declarations and implementations across all sub-controllers to properly reference their associated view instances during setup.

#### ###4 — Control and View Navigator Initialization Reordering
- `src/control/Control_Navigator.cpp`: Reordered constructor instantiations to prevent nullptr access and properly construct the controllers.
- `src/view/View_Navigator.cpp`: Correctly passed sub-controller instances to child views and set the views on their corresponding controllers.
- `src/main.cpp`: Updated to show the parent navigator window (`viewWindow`) rather than showing child views directly.

#### ###5 — Direct Page Index Control
- `src/view/View_Navigator.h/.cpp`: Declared and implemented `setPageIndex(int)` method to allow direct QStackedWidget page changes.
- `src/control/Control_Navigator.h/.cpp`: Updated `switchTab(int)` to directly call `viewWindow->setPageIndex(index)` without emitting intermediary signals.
- `src/view/View_Navigator.cpp`: Removed old redundant signal connections as navigation is now directly controlled by `Control_Navigator`.

#### ###6 — Connection Lifecycle Fix
- `src/control/Login_Control.cpp`: Moved the connection of `loginSubmitted` signal to the `setView` method to guarantee that the view is instantiated and not null when the connection is established.

### Files Modified
| File | Action |
|---|---|
| `src/control/Control_Navigator.h` | Updated — updated switchTab parameter |
| `src/control/Control_Navigator.cpp` | Updated — included headers, reordered instantiations, implemented switchTab |
| `src/control/Login_Control.h` | Updated — declared setView |
| `src/control/Login_Control.cpp` | Updated — implemented setView and moved connection setup |
| `src/control/Profile_Control.h` | Updated — declared setView |
| `src/control/Profile_Control.cpp` | Updated — included Profile_View.h, implemented setView |
| `src/control/Dashboard_Control.h` | Updated — declared setView |
| `src/control/Dashboard_Control.cpp` | Updated — implemented setView |
| `src/view/Profile_View.h` | Updated — added default constructor parameter |
| `src/view/View_Navigator.h` | Updated — declared setPageIndex |
| `src/view/View_Navigator.cpp` | Updated — set views on controllers, implemented setPageIndex, cleaned up connections |
| `src/main.cpp` | Updated — updated main entry point to show viewWindow |
