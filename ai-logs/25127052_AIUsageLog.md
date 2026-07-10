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

---

## Session 7 — Bug Fixes, Vietnamese UI & Feature Overhaul (~22:39)

### Tasks Completed

#### Bug Fix 1: Missing slot declarations (employeeswidget.h)
`filterEmployees(const QString&)` and `applyRoleFilter(int)` were implemented in the `.cpp` but not declared in the header. Fixed by adding all new slot declarations to `employeeswidget.h`.

#### Bug Fix 2: Undefined `logEvent()` (employeeswidget.cpp — lines 335, 362, 415)
`logEvent(...)` was called in three places but never declared or defined anywhere. All three calls removed.

#### Bug Fix 3: Out-of-scope variables on line 335
`row` and `emp` were referenced outside their `for` loop scope. The stray `logEvent` line was removed as part of Bug Fix 2, eliminating this error simultaneously.

#### Bug Fix 4: Missing `showError` / `showSuccess` implementations
Both methods were declared in the header and called by `Employee_Control`, but had no body in the `.cpp`. Implemented using `QMessageBox::critical` and `QMessageBox::information`.

#### Bug Fix 5: Unconnected search bar and filter combo
`searchRoster::textChanged` now wired to `filterEmployees`. Old `filterCombo` replaced entirely by the new filter button (Feature 2 below).

#### Bug Fix 6: Edit/Delete action buttons not connected
Buttons in Col 6 were created but signals were never emitted. Fixed inside `renderTable()` using lambdas capturing `empId` → emit `requestEditEmployee(empId)` and `requestDeleteEmployee(empId)`.

#### Feature 1: Full Vietnamese Language Switch
All visible UI strings switched to Vietnamese across:
- `employeeswidget.cpp` — page title "Quản lý nhân viên", table headers, metric card labels, search placeholder, footer, sort/filter tooltips, add button, status/role badges.
- `AddEmployee_Dialog.cpp` — window title, form labels, buttons, error messages, file dialog.
- `EditEmployee_Dialog.cpp` — window title, form labels, buttons, error messages, file dialog.
- Role getters in both dialogs now map Vietnamese display text ("Nhân viên" / "Quản lý") back to English internal values ("Staff" / "Manager") so the controller/model are unaffected.

#### Feature 2: Filter Button Overhaul
- Replaced `filterCombo` (QComboBox) with an icon-only `filterBtn` (QPushButton).
- New `buildFilterDropdown()` creates a floating `QFrame` child of `EmployeesWidget` with:
  - **Vai trò** section: checkboxes for Nhân viên / Quản lý / Quản trị viên.
  - **Giới tính** section: checkboxes for Nam / Nữ.
- Clicking `filterBtn` toggles the dropdown open/closed.
- Icon switches `filter.svg` ↔ `filter-slash.svg` with dropdown state.
- Checking any box triggers `applyFilter()` → `updateTableVisibility()` (live filtering).
- Closed dropdown preserves all checked states.
- Gender data stored in `Qt::UserRole + 2` on the ID column item for fast lookup.

#### Feature 3: Sort Button Overhaul
- New `buildSortDropdown()` creates a floating `QFrame` with two options: **Sắp xếp theo Mã NV** / **Sắp xếp theo Tên**.
- Sort button toggles the dropdown; selecting an option:
  - Sets `m_sortField` and `m_sortDir = 1` (ascending).
  - Shows `sortTagBar` below the roster header with label "📌 Theo [field]: từ thấp đến cao ✕".
  - Changes sort icon to `sort-from-bottom-to-top.svg`.
  - Calls `applySort()` using `std::stable_sort` on a copy of the cached employee list.
- Clicking the tag label cycles:
  - Ascending → Descending (icon: `sort-from-top-to-bottom.svg`, label: "từ cao đến thấp ✕").
  - Descending → Reset (icon reverts to `sort-vertical-svgrepo-com.svg`, tag bar hidden).
- `loadEmployees` now splits into `loadEmployees` (caches list) + `renderTable` (renders), so `applySort` can re-render a sorted copy without corrupting the cache.

#### Feature 4: Add Employee Dialog Polish
- Removed `inpUsername` and `inpPassword` input fields from the UI form.
- Added an info note: "Tài khoản đăng nhập sẽ được tạo tự động từ số CCCD."
- `getUsername()` auto-returns `inpCitizenId->text()` (citizenId as username).
- `getPassword()` returns `"NhanVien@123"` as a safe default — controller interface unchanged.
- All form labels and buttons switched to Vietnamese.

#### resources.qrc
Registered 4 new SVG files: `filter.svg`, `filter-slash.svg`, `sort-from-bottom-to-top.svg`, `sort-from-top-to-bottom.svg`.

### Files Modified
| File | Action |
|---|---|
| `resources/resources.qrc` | Registered 4 new SVG icons |
| `src/view/employeeswidget.h` | Full rewrite — new members, slot declarations, removed dead code |
| `src/view/employeeswidget.cpp` | Full rewrite — all 6 bugs fixed, Vietnamese, filter/sort dropdowns, action buttons wired |
| `src/view/AddEmployee_Dialog.h` | Removed username/password members, added getUsername/getPassword auto-gen |
| `src/view/AddEmployee_Dialog.cpp` | Removed username/password UI, full Vietnamese |
| `src/view/EditEmployee_Dialog.cpp` | Full Vietnamese, role mapping fix |
| `ai-logs/25127052_AIUsageLog.md` | Updated this log |

---

## Session 8 — Include Refactoring and Precompiled Headers (2026-07-10 ~ 14:55)

### Task 1: Create global precompiled header
**Details:** To optimize compilation time and clean up the codebase, all standard library and Qt library include statements were removed from individual files and consolidated into a single precompiled header. With the recent addition of shift-related files, we expanded the coverage of the global header.

**Fix:**
- Created `src/global.h` containing all necessary `<...>` include statements (e.g., `<QString>`, `<QWidget>`, `<QMessageBox>`, `<cmath>`, `<QDate>`, `<QTime>`, etc.).
- Wrote and executed a script to traverse all `.cpp` and `.h` files in the `src/` directory (including the new `shift_model` and `Shift` files).
- The script removed all individual `#include <...>` lines and injected `#include "global.h"` at the top of each file (after `#pragma once` for headers).
- Updated `CMakeLists.txt` to declare `src/global.h` as a precompiled header using `target_precompile_headers`.

### Files Modified
| File | Action |
|---|---|
| `src/global.h` | Created with all standard and Qt includes |
| `src/**/*.cpp`, `src/**/*.h` | Removed individual `<...>` includes and added `#include "global.h"` |
| `CMakeLists.txt` | Added `target_precompile_headers` |
| `ai-logs/25127052_AIUsageLog.md` | Updated this log with the current date and time |

---

## Session 9 — Schedule Control Implementation & Precompiled Header Tweaks (2026-07-10 ~ 15:20)

### Task 1: Schedule Control and Model Refactoring
**Details:** The team provided a basic `shift.cpp` in `core` and a `shift_model.cpp` in `model`, as well as a UML diagram for `Schedule_Control`. I analyzed these files and created a plan to implement the control layer.
- `Shift_Model` was renamed to `Schedule_Model` to better reflect its function managing collections of shifts.
- Fixed a bug in `checkOverlapping` (which originally queried the wrong table) by rewriting it to check overlaps against an in-memory `QList<Shift>` managed by the controller.
- Created `Schedule_Control.h/.cpp` with a `load()` function to fetch shifts directly from the database and populate the model.
- Added necessary getter methods to the core `Shift` class.
- Forward declared `Schedule_View` and initialized it to `nullptr` to unblock UI development.
- Removed outdated `Shift_Model` files and updated `CMakeLists.txt` to include the new ones.

### Task 2: Standardizing Includes
**Details:** To align the newly created/modified schedule files with the global header architecture implemented in Session 8, standard Qt includes were replaced with the global precompiled header.
- Replaced `<QDate>`, `<QList>`, `<QSqlQuery>`, etc., with `#include "global.h"` in `Schedule_Control.h/.cpp`, `Schedule_Model.h/.cpp`, and `Shift.h/.cpp`.

### Files Modified
| File | Action |
|---|---|
| `src/model/Schedule_Model.h/.cpp` | Renamed from `Shift_Model`; implemented overlap check using in-memory list; used `global.h` |
| `src/core/Shift.h/.cpp` | Added getters for properties; used `global.h` |
| `src/control/Schedule_Control.h/.cpp` | Created with `load()` logic fetching from SQLite DB; used `global.h` |
| `CMakeLists.txt` | Swapped `Shift_Model` out for `Schedule_Model` and added `Schedule_Control` |
| `ai-logs/25127052_AIUsageLog.md` | Updated this log with the current date and time |

---

## Session 10 — Schedule Control Design & Profile Model Password Refactor (2026-07-10 ~ 22:07)

### Task 1: Schedule Control Full Design

**Context:** `Schedule_View` was already implemented with two tables (input table with ComboBoxes, summary table), two buttons ("Thêm" / "Lưu"), and two signals (`requestAddShift`, `requestSaveShift`). `Schedule_Model` had the core DB logic but the controller was a skeleton with no signal wiring.

**Changes:**

#### `Schedule_Control.h` — Full redesign
- Added `currentEmployeeId` (`short int`) and `setEmployeeId()`/`getEmployeeId()`.
- Added `listDays` (`QList<QString>`) for Vietnamese day labels (Thứ 2 – CN).
- Added `openHour` / `closeHour` config fields (default 7–22).
- Added private helpers: `dayStringToDate()` and `buildWeeklySummary()`.
- Declared private slots: `onAddShiftRequested(day, start, end)` and `onSaveShiftRequested()`.
- Included `Schedule_View.h` directly (view is now fully wired).

#### `Schedule_Control.cpp` — Full implementation
- **`setView()`**: connects `requestAddShift` → `onAddShiftRequested` and `requestSaveShift` → `onSaveShiftRequested`.
- **`load()`**: determines if registration is open (Monday rule), calls `setUpDataInputTable()`, fetches this week's shifts via `model->getSchedule()`, and pushes `buildWeeklySummary()` to the view.
- **`onAddShiftRequested()`**: parses day string to `QDate` via `dayStringToDate()`, parses time strings, validates start < end, calls `model->handleAddShiftSubmission()`, shows error on null return, refreshes summary on success.
- **`buildWeeklySummary()`**: iterates `model->getShiftList()`, formats each shift as `"HH:mm – HH:mm"`, returns `QMap<int, QList<QString>>` for the view.
- **`dayStringToDate()`**: finds day index in `listDays`, computes Monday of the current week + offset.

#### `Schedule_Model.h` — Getter added, invalid signals removed
- Added `const QList<QList<Shift*>>& getShiftList() const` (inline getter for controller).
- Removed erroneous `signals:` block (Schedule_Model is not a QObject).

#### `Schedule_Model.cpp` — Bug fix
- **Overlap logic bug fixed**: `handleAddShiftSubmission` was calling `if (checkOverlapping(...))` to block insertion, but `checkOverlapping` returns `true` when the slot is **free** and `false` when blocked. Fixed to `if (!checkOverlapping(...))`.

---

### Task 2: Profile Model — updatePassword Refactor

**Context:** `Profile_Model::updatePassword` was a placeholder that stored passwords in plain text to the wrong table (`PROFILES` instead of `ACCOUNTS`) and performed no verification. `Change_password` (Session 6) already had the correct logic.

**Changes:**

#### `Profile_Model.h`
- Included `"model/Change_password.h"` for `PasswordChangeResult`.
- Changed `updatePassword` signature from `bool updatePassword(id, password)` → `PasswordChangeResult updatePassword(id, oldPassword, newPassword)`.

#### `Profile_Model.cpp`
- **`updatePassword`**: removed the old placeholder body. Now instantiates `Change_password` and delegates entirely to `cp.updatePassword(id, oldPw, newPw)`.
- **`checkIfUserExist`** — bug fixes:
  - Added `Security::hashPassword()` before comparison (was comparing plain text vs stored hash).
  - Added missing `query.next()` call (without it, `query.value()` always returns null).
  - Fixed column name from `"password"` → `"passWord"` (matches ACCOUNTS table schema used by Change_password).
- Added `#include "utils/Security.h"`.

### Files Modified
| File | Action |
|---|---|
| `src/control/Schedule_Control.h` | Full redesign — employeeId, slots, helpers |
| `src/control/Schedule_Control.cpp` | Full implementation — load, add, summary, wiring |
| `src/model/Schedule_Model.h` | Added `getShiftList()` getter; removed invalid signals block |
| `src/model/Schedule_Model.cpp` | Fixed overlap logic inversion bug |
| `src/model/Profile_Model.h` | New updatePassword signature returning PasswordChangeResult |
| `src/model/Profile_Model.cpp` | Delegated to Change_password; fixed checkIfUserExist bugs |
| `ai-logs/25127052_AIUsageLog.md` | Updated this log |

---

### Hotfix 1: Fix `Profile_Control` integration with new password signature
- **Bug:** `Profile_Control::handlePasswordUpdate` and `Profile_View` were still using the old signature (`bool(password)` instead of `PasswordChangeResult(oldPw, newPw)`), causing a compile error and broken logic.
- **Fix:** 
  - Updated `EditPassword_Widget.h/.cpp` to emit `saveRequested(oldPassword, newPassword)`.
  - Updated `Profile_Control.h/.cpp`'s `handlePasswordUpdate` to accept both passwords and return `PasswordChangeResult`. Removed the old bug where it accidentally set the user's name to their password (`setName(password)`).
  - Updated `Profile_View.cpp` lambda to properly validate the confirm password and switch on `PasswordChangeResult` to show exact error messages (wrong old pw, too weak, etc.) using `QMessageBox`.
