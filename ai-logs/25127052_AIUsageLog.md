# AI Usage Log — Student ID: 25127052

## Session Overview

**Date:** 2026-07-03  
**Task:** Iterative improvements to the `EmployeesWidget` Qt C++ dashboard UI + merge bug fix.

---

## Session 1 — Initial Build (~20:00)

Built the full EmployeesWidget dashboard from scratch with: top bar (breadcrumb, clock, bell), 3 metric cards, team roster table with avatar, role/status badges, and action buttons. QSS styled in `styles.qss`. Mock data for 7 employees.

---

## Session 2 — F&B Refinements (~20:22)

1. **QSS reorganized** into 8 named sections in `styles.qss`.
2. **Department column removed** (F&B context).
3. **Base Rate → Monthly Rate** (monthly billing model).
4. **Simplified staff names** (Staff A–D, Manager A–B, Admin).
5. **Right-side info panels** added (Next Shift, Absent Today).

---

## Session 3 — UI Cleanup & Bug Fix (~21:36)

### Changes Made

#### ###1 — Removed Next Shift & Absent Today panels
- `employeeswidget.h`: Removed `nextShiftPanel`, `absentTodayPanel` members and `createInfoPanel()` factory.
- `employeeswidget.cpp`: Removed `createInfoPanel()` function body, removed `contentRowLayout` / right column construction. `rosterCard` now fills the full width directly added to `mainLayout`.
- `styles.qss`: Removed `#nextShiftPanel`, `#absentTodayPanel`, `#nextShiftPanelHeader`, `#absentTodayPanelHeader`, `#infoPanelTitle`, `#infoPanelIcon`, `#infoPanelDivider`, `#infoPanelBody`, `#infoPanelHint` selector blocks.

#### ###2 — Sort/Filter combo next to search bar
- `employeeswidget.h`: Added `QComboBox *filterCombo` member; added `#include <QComboBox>`.
- `employeeswidget.cpp`:
  - Added `filterCombo` construction in `setupUi()`, placed between `searchRoster` and `addEmployeeBtn` in the roster header layout.
  - Options: `All Roles | Staff | Manager | Admin`.
  - Added `applyRoleFilter(int)` slot connected to `filterCombo::currentIndexChanged`.
  - `filterEmployees()` now combines text search AND role filter: each row is hidden unless both match.
  - Row data: role stored in `Qt::UserRole` on the ID column item for fast lookup.
- `styles.qss`: Added `#filterCombo` block (border, radius, hover) + `#filterCombo::drop-down` + `#filterCombo QAbstractItemView`.

#### ###3 — Replaced top bar with Profile Block
- **Removed:** `topBarFrame`, `lblBreadcrumb`, `searchTopBar`, `lblClock`, `clockTimer`, `bellFrame/bellIcon/bellBadge`.
- **Added:** `profileBlock` (QFrame, transparent, 52px tall) containing:
  - Left: `lblPageTitle` ("Staff Management") — bold, 20px.
  - Right: `userCard` (QFrame, white card, rounded, hover effect) containing:
    - Circular `userAvatar` label ("ME", blue #2563EB bg, 36×36).
    - VBox: `userName` ("ME") + `userRole` ("Admin").
    - `userCaret` ("›") caret icon.
- `userCard` has `eventFilter` installed on it in `setupConnections()`. On `MouseButtonRelease`, `EmployeesWidget` emits `profileClicked()`.
- `employeeswidget.h`: Added `signals: void profileClicked()`, `QFrame *profileBlock`, `protected: bool eventFilter()` override. Removed clock/bell members.
- `styles.qss`: Added Section 2 "PROFILE BLOCK" with styles for `#profileBlock`, `#lblPageTitle`, `#userCard`, `#userName`, `#userRole`, `#userCaret`. Removed Section 2 "TOP BAR" (topBarFrame, lblBreadcrumb, searchTopBar, lblClock, bellFrame, bellIcon, bellBadge).

#### ###4 — Fixed merge bug: "invalid use of class View_Navigator"
**Root cause:** After the team merge renamed `Main_View` → `Dashboard_View` and introduced the `Control_Navigator` / `View_Navigator` architecture, the old `main_control.h` still contained:
```cpp
#include "view/main_view.h"  // ← file deleted in merge
```
And `main_control.cpp` still called `Main_View` methods (`switchPage()`, constructor, etc.). Qt's MOC / CMake picked this up and caused cascading "invalid use of class" errors.

**Fix:**
- `src/control/main_control.h` → Replaced with a safe stub that **forward-declares** `Main_View` (no include) and provides an empty no-op `Main_Control` class body. The file is kept (not deleted) to avoid further Git conflicts.
- `src/control/main_control.cpp` → Replaced with a 3-line stub that only includes the fixed header and leaves a comment explaining the migration.

**Architecture note:** All real logic is now in `Control_Navigator` / `Dashboard_Control`. `main_control.h/.cpp` are dead code and can be safely deleted after team alignment.

### Files Modified
| File | Action |
|---|---|
| `src/view/employeeswidget.h` | Updated — removed panels/clock, added profileBlock/filterCombo/eventFilter |
| `src/view/employeeswidget.cpp` | Updated — all 3 UI changes applied |
| `resources/styles/styles.qss` | Updated — Section 2 replaced, right-panel sections removed, filterCombo added |
| `src/control/main_control.h` | Bug fix — replaced broken include with safe stub |
| `src/control/main_control.cpp` | Bug fix — replaced broken method calls with stub |
| `ai-logs/25127052_AIUsageLog.md` | Updated this log |

---

## Session 4 — Hotfixes (~21:48)

### Bug Fix 1: Missing slot declaration
**Error:** `no declaration matches 'void Dashboard_View::on_btnLogout_clicked()'` / `Out-of-line definition of 'on_btnLogout_clicked' does not match any declaration in 'Dashboard_View'`

**Root cause:** Qt's auto-connect naming convention (`on_<objectName>_<signal>`) requires the slot to be declared in the class header. `Dashboard_View.cpp` defined `on_btnLogout_clicked()` but `Dashboard_View.h` had no `private slots:` block at all — likely omitted during the merge that renamed the class from `Main_View` to `Dashboard_View`.

**Fix:** Added `private slots: void on_btnLogout_clicked();` to [`Dashboard_View.h`](src/view/Dashboard_View.h).

### Bug Fix 2: Vtable undefined reference
**Error:** `undefined reference to 'vtable for Main_Control'` in `Login_Control.cpp.obj`

**Root cause:** `Login_Control.cpp` was still trying to instantiate `Main_Control` (`new Main_Control(this)`). Because `Main_Control` is obsolete and wasn't processed properly by the build system (it's not even in `CMakeLists.txt`), trying to link to its constructor and vtable caused a linker error. With `Control_Navigator` handling view switching via the `loginSuccessful` signal anyway, instantiating `Main_Control` is redundant.

**Fix:** Removed the `new Main_Control(this)` code entirely from `Login_Control.cpp`, and removed `#include "main_control.h"`.

### Bug Fix 3: Missing sidebar navigation
**Error:** After refactoring to `Dashboard_View`, clicking the sidebar buttons (Overview, HR, Timekeep, Salary, Report, Settings) did not navigate to their respective pages because the slots were not defined.

**Fix:** Added the necessary `on_btnMenu_<Page>_clicked` slots to `Dashboard_View.h` and implemented them in `Dashboard_View.cpp` to call `ui->stackedWidget->setCurrentWidget(ui->page<Page>)`.

| File | Change |
|---|---|
| `src/view/Dashboard_View.h` | Added `private slots: void on_btnLogout_clicked();` and menu button slots |
| `src/view/Dashboard_View.cpp` | Implemented menu button slots |
| `src/control/Login_Control.cpp` | Removed obsolete references to `Main_Control` |
| `ai-logs/25127052_AIUsageLog.md` | Updated this log |

---

## Session 5 — Build Optimization (~22:10)

### Issue: Very slow compilation times
**Root Cause 1:** The `CMakeLists.txt` contained major duplicates. Several files (`Dashboard_View.h/.cpp/.ui`, `View_Navigator.h/.cpp/.ui`, `Profile_View.h/.cpp/.ui`, `employeeswidget.h`, `Profile_Control.cpp`) were defined in the `PROJECT_SOURCES` variable *and* appended again manually to the `qt_add_executable()` call. `Profile_Control.cpp` was even appended twice in `qt_add_executable()`. This causes CMake to process files multiple times. For `AUTOUIC` and `AUTOMOC`, this means regenerating headers, and compiling `.cpp` files multiple times during the build process, directly leading to massively bloated build times.

**Root Cause 2:** In `.h` files (e.g. `employeeswidget.h`), there is a heavy reliance on direct `#include <Q...>` (17+ Qt includes) instead of forward declarations. While refactoring all includes would be a large architectural change, fixing the CMake issue resolves the most egregious build-time duplication.

**Fix:** Cleaned up `CMakeLists.txt` to remove all files from `qt_add_executable` that were already included in `PROJECT_SOURCES`.

| File | Change |
|---|---|
| `CMakeLists.txt` | Removed duplicate source definitions from `qt_add_executable` |

---

## Session 6 — UI Refinements & Password Model Implementation (~13:20)

### Task 1: UI Icon Replacements and Clean Up
**Details:** The `employeeswidget` originally used emojis for action buttons and metric cards. We needed to replace these with scalable vector graphics (SVGs) for a more professional and consistent look, and remove the emojis to simplify the codebase.

**Fix:** 
- Removed emojis from `employeeswidget.cpp` and `employeeswidget.h`.
- Registered new SVG icons (`dolar-svgrepo-com.svg`, `edit-svgrepo-com.svg`, `people-svgrepo-com.svg`, `warning-circle-svgrepo-com.svg`, `trash-bin-trash-svgrepo-com.svg`, and `sort-vertical-svgrepo-com.svg`) in `resources.qrc`.
- Updated `createMetricCard` and `createActionButton` to dynamically load `QPixmap` and `QIcon` when image paths are passed.
- Added a new sort button (`sortBtn`) next to the filter combo box in the roster header.
- Cleaned up obsolete `.png` files from the filesystem.

| File | Change |
|---|---|
| `src/view/employeeswidget.h` | Removed emoji parameters; added `sortBtn` |
| `src/view/employeeswidget.cpp` | Replaced strings with SVG paths; added QIcon/QPixmap logic; added sort button to layout |
| `resources/resources.qrc` | Added new SVG files; removed obsolete PNG files |
| `resources/images/*` | Deleted old `.png` files |

### Task 2: Change Password Model Implementation
**Details:** Needed a new model to handle the logic of updating a user's password securely (hashing, verifying old password, saving to DB).

**Fix:**
- Designed and implemented the `Change_password` class.
- Added an enum `PasswordChangeResult` to clearly communicate status (success, wrong old password, weak new password, database error).
- Implemented `verifyOldPassword`, `validatePasswordStrength`, and `executePasswordUpdate` methods relying on SQLite and `Security::hashPassword`.
- Added the new model files to `CMakeLists.txt` for compilation.

| File | Change |
|---|---|
| `src/model/Change_password.h` | Created class structure and result enum |
| `src/model/Change_password.cpp` | Implemented business logic for password updates |
| `CMakeLists.txt` | Appended new source files to `PROJECT_SOURCES` |
