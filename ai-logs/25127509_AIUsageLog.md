# AI Usage Log — Student: Trần Quang Thịnh (ID: 25127509)

## Session Overview

**Date:** 2026-06-26 to 2026-07-25  
**Task:** Implementation of Core Login Logic, Add/Edit Employee validations and architectural decoupling, building the View Schedule feature (UI, Control, and Model integrations), and incorporating the Schedule Optimizer using Min-Cost Max-Flow algorithms.

---

## Session 1 — Core Login Logic & Database Skeleton (~22:55, June 26)

### Changes Made
- **Core MVC Setup:** Implemented the foundational MVC skeleton for the Login feature by building `Login_View`, `Login_Control`, and `Login_Model`.
- **Database Linkage:** Fixed errors in `database.cpp` and successfully connected the Login UI state to the `verifyLogin` function inside `Login_Model` for user authentication.
- **Resource Management:** Restructured `resources.qrc` and added necessary assets (`eyeClosed.svg`, `login_bg.jpg`, fonts) and wrote the initial QSS definitions in `styles.qss`.

### Files Modified
| File | Action |
|---|---|
| `src/model/Login_Model.cpp` | Updated — Implemented `verifyLogin` with SQL |
| `src/control/Login_Control.h/.cpp` | [NEW] Created logic to connect View and Model |
| `src/view/Login_View.ui/.cpp` | Updated — Designed and linked initial Login UI |
| `src/utils/Database.cpp` | Updated — Fixed database skeleton and SQL connections |

---

## Session 2 — Employee Management Dialogs & MVC (~20:36, July 5)

### Changes Made
- **Employee Dialogs:** Developed `AddEmployee_Dialog` and `EditEmployee_Dialog` from scratch to handle full CRUD operations for staff members, including avatar uploads and deletions.
- **Employee MVC Pipeline:** Created `Employee_Model` to interact with the database and `Employee_Control` to mediate data flow between the view and the database, ensuring separation of concerns.
- **Widget Binding:** Linked the new dialogs to the `employeeswidget` UI list.

### Files Modified
| File | Action |
|---|---|
| `src/view/AddEmployee_Dialog.h/.cpp` | [NEW] Built dialog for adding new employees |
| `src/view/EditEmployee_Dialog.h/.cpp` | [NEW] Built dialog for modifying employees |
| `src/model/Employee_Model.h/.cpp` | [NEW] Implemented database logic for employees |
| `src/control/Employee_Control.h/.cpp` | [NEW] Created controller layer |

---

## Session 3 — Employee Gender Field Extension (~11:52, July 12)

### Changes Made
- **Gender Field Integration:** Added a "Gender" selection combo box to both `EditEmployee_Dialog` and `EditProfile_Widget` to allow for more complete employee profiling.
- **Model & Database Update:** Updated the underlying `User` core class and `Employee_Model` logic to properly serialize and deserialize the new gender attribute to/from the SQLite database.
- **UI Layout Fixes:** Adjusted the geometry and layout constraints inside the employee dialogs to ensure the UI does not break when the new gender field is populated.

### Files Modified
| File | Action |
|---|---|
| `src/view/EditEmployee_Dialog.h/.cpp` | Updated — Added gender UI logic and value retrieval |
| `src/view/EditProfile_Widget.h/.cpp` | Updated — Added gender UI field for profile editing |
| `src/core/User.h/.cpp` | Updated — Added `gender` attribute and getter/setters |
| `src/model/Employee_Model.cpp` | Updated — Bound gender data to SQL UPDATE queries |

---

## Session 4 — Centralized Validation & Widget Scaling (~21:02, July 16)

### Changes Made
- **Centralized Validation Layer:** Created a dedicated `Validator` class containing regex and length checks to validate employee data (phone numbers, emails, required fields) preventing malformed data entry.
- **MVC Validation Linking:** Integrated the `Validator` into `AddEmployee_Dialog` (View) for immediate visual feedback and `Employee_Control` (Controller) to block invalid database insertions.
- **Responsive UI Scaling:** Fixed a bug in `employeeswidget` where employee avatars and icons were unscaled. Ensured SVG icons and widget elements dynamically scale when the parent window resizes.

### Files Modified
| File | Action |
|---|---|
| `src/model/Validator.h/.cpp` | [NEW] Created robust data validation logic |
| `src/view/AddEmployee_Dialog.h/.cpp` | Updated — Hooked UI events to data validation |
| `src/control/Employee_Control.cpp` | Updated — Added server-side validation checks |
| `src/view/employeeswidget.cpp` | Updated — Fixed icon and avatar aspect-ratio scaling |

---

## Session 5 — Login UI Overhaul (~15:55, July 19)

### Changes Made
- **Aesthetic Redesign & Custom QSS:** Completely overhauled the `Login_View` to feature a modern, professional aesthetic. Wrote extensive QSS (Qt StyleSheet) rules in `styles.qss` targeting the login components (e.g., configuring `border-radius` for rounded corners, hover states for buttons, and transparent/gradient backgrounds).
- **Layout Restructuring:** Restructured the underlying `Login_View.ui` layouts (utilizing QVBoxLayout and QHBoxLayout) to properly accommodate the new UI design and center the login container.
- **Visual Assets:** Imported and implemented a new background graphic (`login_bg.png`) via the Qt Resource System (`resources.qrc`), scaling it to cover the application window smoothly.

### Files Modified
| File | Action |
|---|---|
| `src/view/Login_View.ui` | Updated — Redesigned layout structure (VBox/HBox) for the new UI |
| `src/view/Login_View.cpp` | Updated — Adjusted UI references and integrated signal/slot mappings |
| `resources/styles/styles.qss` | Updated — Applied detailed stylesheet properties (borders, hovers) |
| `resources/images/login_bg.png` | [NEW] Added high-quality background asset |

---

## Session 6 — View Schedule Infrastructure & ShiftBlock (~21:50, July 23)

### Changes Made
- **View Schedule UI (`viewschedule_view`):** Built a dedicated UI from scratch for managers and employees to visually inspect schedules. Designed a tabular/grid interface where columns represent days of the week and rows represent time slots.
- **ShiftBlock Data Structure:** Engineered the core `ShiftBlock` struct designed specifically to encapsulate UI-facing schedule data (time-bounds, employee IDs, shift-types, status). This struct bridges the gap between raw database tuples and renderable UI elements.
- **Controller-Model Pipeline:** Implemented `viewschedule_control` to mediate interactions. Updated `Schedule_Model` to query assigned shifts from the database, translating SQL records into collections of `ShiftBlock` objects, which are then passed to the view via the controller.

### Files Modified
| File | Action |
|---|---|
| `src/view/viewschedule_view.h/.cpp/.ui` | [NEW] Created the timetable grid UI for Schedule viewing |
| `src/control/viewschedule_control.h/.cpp` | [NEW] Implemented logic to mediate Schedule interactions |
| `src/core/ShiftBlock.h` | [NEW] Created foundational ShiftBlock data structure for UI rendering |
| `src/model/Schedule_Model.h/.cpp` | Updated — Added SQL SELECT logic to load schedules dynamically |
| `src/view/Schedule_View.cpp` | Updated — Hooked the new view to the main application navigator |

---

## Session 7 — MVC Decoupling, App Branding & Optimizer Integration (~14:45 - ~18:31, July 25)

### Changes Made
- **App Branding & MVC Refactoring:** Added the official application logo (`logo.png`) and decoupled `AddEmployee_Dialog` from directly querying `Employee_Model`, shifting logic to `Employee_Control` to strictly adhere to MVC.
- **Min-Cost Max-Flow Algorithm (SPFA):** Fully integrated the automated scheduling algorithm (`Optimizer.cpp`). Implemented a graph network structured as `Source -> Employees -> Days -> Slots -> Sink`. The algorithm uses the Shortest Path Faster Algorithm (SPFA) to find optimal shift assignments.
- **Cost Function Constraints:** Engineered the flow logic to prioritize fairness: the cost metric uses `userMinutes[currentUser] / HOUR_SCALE` to prioritize employees with fewer working hours, and applies a `WEEKDAY_PENALTY` to ensure weekend shifts are filled correctly. The flow capacity is dynamically constrained by `Config::getMaxStaffPerShift()`.
- **End-to-End Pipeline & UI Population:** Routed the outputs of the `Optimizer` to directly update the statuses (`s->setStatus(1)`) of generated shifts. `Schedule_Model` persists these assignments into `Systems.db`, while `Schedule_View` reads and populates the timetable GUI with the optimized allocations.

### Files Modified
| File | Action |
|---|---|
| `src/view/AddEmployee_Dialog.cpp`, `src/model/Employee_Model.cpp` | Updated — Refactored to enforce strict MVC |
| `src/core/Optimizer.h/.cpp` | Updated — Implemented SPFA Min-Cost Max-Flow scheduling algorithm |
| `src/model/Schedule_Model.cpp` | Updated — Saved optimizer-generated shifts to SQL |
| `src/view/Schedule_View.cpp` | Updated — Parsed optimized shift data for UI rendering |
| `src/control/Schedule_Control.cpp` | Updated — Mediate algorithm execution and broadcast updates |