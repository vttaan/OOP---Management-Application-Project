# AI Usage Log — Student ID: 25127052

## Session Overview

**Date:** 2026-07-03  
**Task:** Redesign the `EmployeesWidget` UI in a Qt C++ project to match a modern Employee Management System Dashboard specification.

---

## Session 1 — Initial Build (2026-07-03 ~20:00)

### What Was Done

#### Project Exploration & Analysis
- Read and analyzed the full project structure: `src/view/`, `resources/styles/styles.qss`, `src/view/main_view.ui`, `src/view/main_view.cpp`, `src/view/employeeswidget.h/.cpp/.ui`.
- Identified that the project uses **Qt Widgets (C++)**, not React/Tailwind as the prompt mentioned — adapted the implementation to Qt accordingly.
- Studied the existing `styles.qss` to understand color palette, font conventions, and styling patterns already used in the project.
- Confirmed the `EmployeesWidget` is injected into `main_view.cpp`'s `pageHR` panel.

#### `employeeswidget.h` — Full Rewrite
- Replaced old flat `QSqlTableModel`/`QTableView` approach.
- Added rich new members: `QTableWidget`, `QTimer` (for real-time clock), top bar frame, metric card frames, roster card, search bars, footer label.
- Declared helper methods: `createAvatar()`, `createMetricCard()`, `createStatusBadge()`, `createRoleBadge()`, `createActionButton()`.

#### `employeeswidget.cpp` — Full Rewrite
- **Top Bar:** Breadcrumb (`CorpHQ › Employees`), pill search bar, real-time clock, bell icon with "2" badge.
- **Metric Cards (3):** Total Expected Payroll ($466K), Active Staff Clocked In (5/7), Pending Absences (2).
- **Team Roster Table (QTableWidget, 8 cols):** ID, Name+Avatar, Department, Role badge, Pay Type, Base Rate, Status badge, Edit/Delete actions.
- **Mock Data:** 7 employees across Operations, Engineering, Support, Design, HR.

#### `resources/styles/styles.qss` — EmployeesWidget section overhauled
- Replaced dark VS Code theme with light slate dashboard palette.
- New styles for topBarFrame, metricCard, rosterCard, employeesTable, badges, scrollbars.

#### Files Modified
| File | Action |
|---|---|
| `src/view/employeeswidget.h` | Full rewrite |
| `src/view/employeeswidget.cpp` | Full rewrite |
| `src/view/employeeswidget.ui` | Replaced with minimal shell |
| `resources/styles/styles.qss` | EmployeesWidget section replaced |
| `ai-logs/25127052_AIUsageLog.md` | Created this log |

---

## Session 2 — F&B Refinements (2026-07-03 ~20:22)

### Changes Requested
1. Move all inline QSS from `.cpp` into `styles.qss`, organized by function/section.
2. Remove the Department column (F&B context has no need for it).
3. Change Base Rate to Monthly Rate (monthly billing model).
4. Simplify staff names (Staff A–D, Manager A–B, Admin).
5. Add right-side info panels: **Next Shift** and **Absent Today** (placeholder layout, no data yet).

### What Was Done

#### `employeeswidget.h` — Updated
- Added new members: `contentRowLayout` (main content row), `nextShiftPanel`, `absentTodayPanel` (right-side panels).
- Added `createInfoPanel()` factory method declaration.
- Removed no-longer-needed `setupMetricCards()` declaration.
- Cleaned up redundant includes (`QPixmap`, `QPainter`, `QStandardItemModel`, `QSortFilterProxyModel`).

#### `employeeswidget.cpp` — Updated

**###1 QSS Organization:**  
Moved all `setStyleSheet()` calls that could go to named-object QSS into `styles.qss`. Only **per-instance colour overrides** remain inline in C++ (avatar background, metric icon background, role/status badge colours) — these cannot be expressed in static QSS because each instance requires a unique colour token.

**###2 Department column removed:**  
- `setColumnCount()` changed from 8 → 7.
- `setupTableHeader()` updated: headers now `{ID, NAME, ROLE, PAY TYPE, MONTHLY RATE, STATUS, ACTIONS}`.
- `populateTable()` struct removed `department` field; column index mapping adjusted accordingly.

**###3 Monthly Rate:**  
- `EmployeeRecord` struct: `baseRate` field renamed to `monthlyRate`.
- Data values changed from hourly/annual to monthly amounts (e.g., `$28.50/hr` → `$2,280/mo`, `$87K/yr` → `$4,200/mo`).
- Column header: `"BASE RATE"` → `"MONTHLY RATE"`.
- Metric card: `"Total Expected Payroll"` / `"$466K"` → `"Total Monthly Payroll"` / `"$46.6K"`.
- Breadcrumb changed: `CorpHQ › Employees` → `CorpHQ › Staff`.

**###4 Simplified Staff Names:**

| Old Name | New Name | ID |
|---|---|---|
| James Carter | Staff A | EMP-1042 |
| Sarah Mitchell | Manager A | EMP-1043 |
| Priya Nair | Staff B | EMP-1044 |
| Liam Johnson | Staff C | EMP-1045 |
| Aisha Rahman | Staff D | EMP-1046 |
| Tom Nguyen | Manager B | EMP-1047 |
| Emily Chen | Admin | EMP-1048 |

**###5 Right-side Info Panels:**  
- Layout changed: `mainLayout` now contains a `contentRowLayout (QHBoxLayout)`.
  - Left side: `rosterCard` with stretch factor 3.
  - Right side: `QVBoxLayout` containing `nextShiftPanel` and `absentTodayPanel`, each with stretch factor 1.
- `createInfoPanel(title, iconEmoji, objectName)` factory: builds a card with a coloured header bar (blue for Next Shift, soft red for Absent Today), a thin separator, and an empty body with a "— No data —" placeholder hint in the centre.

#### `resources/styles/styles.qss` — Restructured

Completely reorganized the EmployeesWidget block into **8 clearly labelled sections** with banner comments:

```
1. ROOT WIDGET
2. TOP BAR (breadcrumb, search, clock, bell)
3. METRIC SUMMARY CARDS
4. ROSTER CARD (header, search, add-btn)
5. STAFF TABLE (table, header view, name label, action buttons)
6. TABLE FOOTER
7. RIGHT INFO PANELS (Next Shift / Absent Today)
8. SHARED SCROLLBARS
```

New QSS rules added for:
- `#tableActionBtn` — action icon buttons (previously only inline in C++)
- `#nextShiftPanel`, `#absentTodayPanel` — right panel card shells
- `#nextShiftPanelHeader`, `#absentTodayPanelHeader` — coloured header bars
- `#infoPanelTitle`, `#infoPanelIcon`, `#infoPanelDivider`, `#infoPanelBody`, `#infoPanelHint` — shared panel internals

Also added inline comments in `.qss` explaining which styles must remain in C++ (per-instance colours for avatars, badges, and metric icons).

#### Files Modified
| File | Action |
|---|---|
| `src/view/employeeswidget.h` | Updated — new panel members & factory |
| `src/view/employeeswidget.cpp` | Updated — all 5 changes applied |
| `resources/styles/styles.qss` | EmployeesWidget section fully restructured (8 sections) |
| `ai-logs/25127052_AIUsageLog.md` | Updated this log |

---

## AI Tools Used
- **Antigravity IDE (Claude Sonnet 4.6 Thinking)** — code generation, file reading/writing, planning
- `view_file`, `list_dir` — for codebase exploration
- `write_to_file`, `replace_file_content`, `run_command` — for all file edits

---

*Log written by AI assistant on behalf of student 25127052.*
