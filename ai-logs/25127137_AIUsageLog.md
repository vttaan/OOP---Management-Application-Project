# AI Usage Log — Student ID: 25127137

## Session Overview

**Date:** 2026-07-02 to 2026-07-07  
**Task:** Implementation of Profile View, Edit Profile popup widget, SessionManager global sharing, and avatar persistence bugfixes.

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

---

## Session 6 — Profile Pop-up, Session Manager, and Git Resolution (~15:49, July 5 to ~00:16, July 6)

### Changes Made

#### ###1 — Profile Edit Pop-up Widget
- `src/view/EditProfile_Widget.h/.cpp` [NEW]: Created the `EditProfile_Widget` class as a sliding/pop-up pane for modifying profile information.
- `src/model/Profile_Model.h/.cpp` [NEW]: Created `Profile_Model` class to handle profile update database queries.
- `src/control/Profile_Control.h/.cpp`: Added `handleProfileUpdate` to bind `EditProfile_Widget` changes to the `Profile_Model` database action.
- `src/view/Profile_View.ui`: Removed vertical stretch on `cardGeneralLayout` to prevent layout shrinking and button clipping, ensuring `btnEditInfo` displays properly.
- `resources/styles/styles.qss`: Added custom stylesheet styling for `btnEditInfo` and fixed scrollbar closing brace syntax error.

#### ###2 — Custom Password Line Edit
- `src/view/EditPassword_Widget.h/.cpp`: Developed `password_LineEdit` as a public inheritor of `QLineEdit` to support visibility toggle icons. Fixed undefined reference error by defining the constructor and initializing the `toggleIcon`.

#### ###3 — SessionManager Integration
- `src/utils/SessionManage.h/.cpp`: Implemented global session memory. Fixed memory leak by deleting `currentUser` on `clearInfo()`.
- `src/control/Control_Navigator.cpp`: Switched to a shared global `SessionManager` pointer to avoid localized `User*` variables. Fixed a segmentation fault by initializing `viewWindow` before referencing it and resolved a memory leak in the destructor by deleting `currentSession`.
- `src/control/Login_Control.cpp`: Passed the global `currentSession` pointer to share memory upon successful login.

#### ###4 — Qt MOC and Signal Connection Fixes
- `src/control/Dashboard_Control.h`: Removed `const` qualifiers from Qt signals to resolve moc compiler and linking issues.
- `src/view/Dashboard_View.h/.cpp`, `src/view/Profile_View.h/.cpp`, `src/view/Login_View.h/.cpp`: Added and implemented `setController` methods to establish proper bindings between controllers and views.
- `src/control/Dashboard_Control.cpp`, `src/control/Profile_Control.cpp`, `src/control/Login_Control.cpp`: Bound controllers to views inside their respective `setView` methods to resolve stack crash at `logoutSubmitted` signal.

#### ###5 — CMake Build & Git Conflict Resolution
- `CMakeLists.txt`: Cleaned up git conflict markers, removed obsolete `employeeswidget` sources, and registered `Profile_Model` and `EditProfile_Widget` files.
- `resources/styles/styles.qss`, `src/view/Dashboard_View.cpp`, `src/control/Login_Control.cpp`: Manually resolved all git conflicts after executing a `git stash`.

### Files Modified
| File | Action |
|---|---|
| `src/model/Profile_Model.h` | [NEW] Created Profile_Model header |
| `src/model/Profile_Model.cpp` | [NEW] Created Profile_Model source file |
| `src/view/EditProfile_Widget.h` | [NEW] Created EditProfile_Widget header |
| `src/view/EditProfile_Widget.cpp` | [NEW] Created EditProfile_Widget source file |
| `src/control/Profile_Control.h` | Updated — added update handler |
| `src/control/Profile_Control.cpp` | Updated — implemented profile update bindings |
| `src/view/Profile_View.ui` | Updated — resolved stretch clipping bug |
| `resources/styles/styles.qss` | Updated — added popup button styling and resolved git conflict |
| `src/view/EditPassword_Widget.h` | Updated — defined missing constructors and icon initialization |
| `src/utils/SessionManage.cpp` | Updated — resolved currentUser memory leak |
| `src/control/Control_Navigator.cpp` | Updated — integrated SessionManager, fixed segfault and memory leak |
| `src/control/Dashboard_Control.h` | Updated — removed const qualifiers from signals |
| `src/control/Dashboard_Control.cpp` | Updated — bound controller to view in setView |
| `src/view/Dashboard_View.h` | Updated — added setController declaration |
| `src/view/Dashboard_View.cpp` | Updated — implemented setController and resolved git conflict |
| `src/view/Profile_View.h` | Updated — added setController declaration |
| `src/view/Profile_View.cpp` | Updated — implemented setController |
| `src/view/Login_View.h` | Updated — added setController declaration |
| `src/view/Login_View.cpp` | Updated — implemented setController |
| `src/control/Login_Control.cpp` | Updated — bound controller to view and resolved git conflicts |
| `CMakeLists.txt` | Updated — registered new files and resolved git conflicts |

---

## Session 7 — Avatar Update Bugfixes (~09:23 to ~09:29, July 7)

### Changes Made

#### ###1 — Avatar Database Persistence Fix
- `src/model/Profile_Model.cpp`: Updated the profile SQL query to include and bind `avatarPath` parameter so that avatar changes are saved.
- `src/control/Profile_Control.cpp`: Updated `avatarPath` assignments to use the local avatar file name returned by `saveAvatarLocally` rather than the absolute path.

#### ###2 — parameter count mismatch SQL Error
- `src/model/Profile_Model.cpp`: Resolved query execution error by fixing a spelling typo in citizen identity ID column name (`IdCitizenIdentity`).

#### ###3 — Avatar Selection Logic Simplification
- `src/view/EditProfile_Widget.cpp`: Simplified `onChangeAvatarClicked` to store the selected absolute path locally during interaction, postponing the file copy step until the save profile action is confirmed to prevent premature file copying.

### Files Modified
| File | Action |
|---|---|
| `src/model/Profile_Model.cpp` | Updated — added avatarPath to query, fixed column spelling typo |
| `src/control/Profile_Control.cpp` | Updated — updated avatarPath parameter binding |
| `src/view/EditProfile_Widget.cpp` | Updated — postponed avatar copying to prevent premature state changes |

