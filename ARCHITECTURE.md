# Optimus Management Application - Technical Documentation

## 1. System Architecture & Overview

### Core Purpose
The Optimus Management Application is a desktop-based Human Resources and Employee Management System built in C++17 using the Qt 6 Framework. It facilitates managing employee profiles, payroll, shift scheduling, and user authentication with role-based access control (RBAC).

### Design Patterns
* **Model-View-Controller (MVC):** The overarching architecture. It strictly separates the graphical user interface (Views in Qt), data manipulation and SQLite interactions (Models), and business logic routing (Controllers). *Why:* Ensures decoupled code, making it easy to swap out UI components without breaking database logic.
* **Singleton:** Used for `Database` and `SessionManager`. *Why:* There should only ever be one active connection to the SQLite database and one global session state tracking the currently logged-in user to prevent race conditions and memory overhead.
* **Factory Method:** Used in `UserFactory`. *Why:* Abstracts the instantiation of specific user roles (`Staff` vs. `Manager`), allowing the system to easily instantiate polymorphic `User` objects directly from raw database query strings without cluttering the models.

### Workflow
1. The application boots and initializes the `Control_Navigator` (Master Controller).
2. The user interacts with a View (e.g., clicks "Login" on `Login_View`).
3. The View emits a Qt Signal containing the raw input data.
4. A specific Controller (e.g., `Login_Control`) catches the signal and instantiates a Model (e.g., `Login_Model`).
5. The Model queries the `Database` Singleton, fetches data, and uses the `UserFactory` to generate domain entities (e.g., `User`).
6. The Model returns the entity to the Controller, which updates the `SessionManager` Singleton and signals the Master Controller to swap the active View.

---

## 2. Class-Level Documentation

### A. `User` (Abstract Base Class)
* **Responsibility:** Represents the foundational data entity of a person in the system. It holds core demographic and identity data but relies on derived classes (`Staff`, `Manager`) for role-specific logic (like salary calculation).
* **Internal State:**
  * `QString role, avatarPath, idCitizenIdentity, name, dob, address, phoneNum, gender;`
  * `short int idEmployee;`

### B. `UserFactory`
* **Responsibility:** A stateless factory class responsible for instantiating concrete derivatives of the `User` class based on raw database parameters.
* **Internal State:** None (Stateless/Utility class containing only static methods).

### C. `Database`
* **Responsibility:** Manages the active SQLite database connection. It ensures queries are executed safely against `database/Systems.db`.
* **Internal State:**
  * `static Database* instance;` (The Singleton pointer)
  * `QSqlDatabase dbConnect;` (The active Qt database connection object)

### D. `SessionManager`
* **Responsibility:** Holds the global state of the currently authenticated user and provides role-based permission checks across the application.
* **Internal State:**
  * `User* currentUser;` (Pointer to the active user entity)

### E. `Login_Model`
* **Responsibility:** Handles the data-layer logic for authentication. Validates credentials against the database and fetches the user's profile data upon success.
* **Internal State:** None (Instantiated locally by Controllers as needed).

---

## 3. Function-Level Documentation (Deep Dive)

### Class: `UserFactory`

| Signature | Purpose | Logic & Algorithm | State Mutation | Exceptions / Edge Cases | Complexity |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `static User* createContainsUser(QString r, short int idEmp, ...)` | Instantiates a user object from existing database records. | Checks the `r` (role) string. If `"Manager"`, returns `new Manager(...)`. If `"Staff"`, returns `new Staff(...)`. | None (Stateless). | Returns `nullptr` if the role string is unrecognized. | Time: $O(1)$<br>Space: $O(1)$ |
| `static User* createNewUser(QString r, ...)` | Creates a brand new user and auto-increments the ID. | Executes `SELECT MAX(idEmployee)` from the DB. Increments the retrieved ID by 1. Passes the new ID to the respective `Staff` or `Manager` constructor. | Modifies database connection state implicitly via query execution. | Returns `nullptr` if the role string is invalid. Fails gracefully if the query fails (defaults ID to 1). | Time: $O(1)$ (DB index lookup)<br>Space: $O(1)$ |

### Class: `Database`

| Signature | Purpose | Logic & Algorithm | State Mutation | Exceptions / Edge Cases | Complexity |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `static Database* getInstance()` | Retrieves the global Singleton instance. | Checks if `instance` is null. If null, calls the private constructor to initialize `dbConnect`. Returns the pointer. | Mutates the static `instance` pointer on first call. | None. Thread-safe initialization not explicitly implemented, relies on single-threaded Qt UI event loop. | Time: $O(1)$<br>Space: $O(1)$ |
| `QSqlQuery execQuery(const QString& query)` | Executes a raw SQL query string and logs errors. | Instantiates a `QSqlQuery`. Calls `.exec()`. If it fails, logs `lastError().text()` to QDebug. | Modifies internal `QSqlDatabase` state by executing statements. | Handles SQL syntax errors by logging, but returns an invalid `QSqlQuery` object which the caller must check. | Time: $O(N)$ (Dependent on DB operation)<br>Space: $O(1)$ |

### Class: `SessionManager`

| Signature | Purpose | Logic & Algorithm | State Mutation | Exceptions / Edge Cases | Complexity |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `void saveCurrentInfo(User* user)` | Sets the globally authenticated user. | Checks if `currentUser` is not null; if so, `delete`s the old pointer to prevent memory leaks. Assigns `currentUser = user`. | Overwrites the `currentUser` pointer. Deallocates previous memory. | Passing a `nullptr` effectively clears the session but is valid. | Time: $O(1)$<br>Space: $O(1)$ |
| `bool checkPermission(const QString& requiredRole) const` | Verifies if the active user matches a required role. | Checks if `currentUser` is null (returns false). Otherwise, compares `currentUser->getRole() == requiredRole`. | None (Read-only `const` method). | Returns `false` immediately if no user is logged in. | Time: $O(1)$<br>Space: $O(1)$ |

### Class: `Login_Model`

| Signature | Purpose | Logic & Algorithm | State Mutation | Exceptions / Edge Cases | Complexity |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `User* verifyLogin(const QString& userName, const QString& password)` | Authenticates credentials and builds a `User` entity. | 1. Fetches DB instance.<br>2. Hashes the input `password` via `Security::hashPassword`.<br>3. Runs prepared statement `SELECT * FROM ACCOUNTS WHERE userName = :u AND passWord = :v`.<br>4. If match, grabs `idEmployee`.<br>5. Queries `PROFILES` table for user details.<br>6. Uses `UserFactory` to return the initialized `User*`. | None. | Returns `nullptr` if the username/password combination is wrong, or if the `PROFILES` lookup fails for the matched ID. | Time: $O(1)$ (Indexed lookup)<br>Space: $O(1)$ |

---

## 4. Usage Example

The following code snippet demonstrates how a developer would programmatically authenticate a user, update the global session, and perform a role-based action using the core architecture described above.

```cpp
#include "model/Login_Model.h"
#include "utils/SessionManage.h"
#include "core/User.h"
#include <QDebug>

void simulateAuthenticationWorkflow() {
    // 1. Controller instantiates the Model to perform the database verification
    Login_Model authModel;
    
    // 2. The Model interacts with the Database and UserFactory internally
    User* authenticatedUser = authModel.verifyLogin("admin", "admin123");
    
    if (authenticatedUser != nullptr) {
        // 3. Login successful. Store the User in the global SessionManager
        SessionManager* session = SessionManager::getInstance();
        session->saveCurrentInfo(authenticatedUser);
        
        qDebug() << "Welcome," << session->getCurrentUser()->getName();
        
        // 4. Perform Role-Based Access Control (RBAC) checks
        if (session->checkPermission("Manager")) {
            qDebug() << "Access Granted: Opening Manager Dashboard...";
            // Emit signal to switch to Manager View...
        } else {
            qDebug() << "Access Granted: Opening Staff Dashboard...";
            // Emit signal to switch to Staff View...
        }
    } else {
        // Handle failure
        qDebug() << "Authentication Failed: Invalid credentials.";
    }
}
```
## 5. Source Code Detail (src)

### 5.1. Control Module (`src/control`)

This module acts as the intermediary between the View and Model layers, orchestrating application flow and data binding.

#### 5.1.1. `Control_Navigator`
*   **Role:** The master controller responsible for managing and routing between all other controllers and their respective views in the application. It acts as the central hub.
*   **Variables:**
    *   `loginController`, `profileController`, `dashboardController`, `employeeController`, `scheduleController`, `salaryController`, `viewScheduleController` (Pointers to sub-controllers). *Called in `main.cpp` or initialized internally.*
    *   `viewWindow` (Pointer to `View_Navigator`). *Called during setup to link views.*
    *   `currentSession` (Pointer to `SessionManager`). *Used to track global login state.*
*   **Functions:**
    *   `switchTab(int index)`: Changes the active view in the application. *Called by `sidebar_widget` or internal logic when navigating.*

#### 5.1.2. `Dashboard_Control`
*   **Role:** Manages the logic for the Dashboard page.
*   **Variables:**
    *   `view` (Pointer to `Dashboard_View`).
    *   `currentSession` (Pointer to `SessionManager`).
*   **Functions:**
    *   `init()`: Initializes connections. *Called by `Control_Navigator`.*
    *   `getView()`, `setView()`: Accessors for the view. *Called during setup.*

#### 5.1.3. `Employee_Control`
*   **Role:** Handles all logic related to employee management (CRUD operations, filtering, sorting).
*   **Variables:**
    *   `m_view` (Pointer to `EmployeesWidget`).
    *   `m_model` (Pointer to `Employee_Model`).
*   **Functions:**
    *   `handleLoadEmployees()`: Fetches employee data from the model. *Calls `Employee_Model::loadData`.*
    *   `handleAddEmployee()`, `handleEditEmployee(int idEmployee)`, `handleDeleteEmployee(int idEmployee)`: CRUD handlers. *Calls respective functions in `Employee_Model` and shows dialogs like `AddEmployee_Dialog`.*
    *   `handleUpdate(...)`: Processes search/filter criteria.

#### 5.1.4. `Login_Control`
*   **Role:** Manages authentication logic, bridging the login UI with the backend authentication process.
*   **Variables:**
    *   `view` (Pointer to `Login_View`).
    *   `currentSession` (Pointer to `SessionManager`).
*   **Functions:**
    *   `handleLoginSubmission(const QString& username, const QString& password)`: Processes the login attempt. *Called by `Login_View::loginSubmitted`. Calls `Login_Model::verifyLogin` and `SessionManager::saveCurrentInfo`.*

#### 5.1.5. `Profile_Control`
*   **Role:** Manages the user profile logic, handling profile updates and password changes.
*   **Variables:**
    *   `view` (Pointer to `Profile_View`).
    *   `model` (`Profile_Model` instance).
    *   `currentSession` (Pointer to `SessionManager`).
*   **Functions:**
    *   `loadUserData()`: Loads user data for display. *Calls `Profile_View::loadUserData`.*
    *   `handleProfileUpdate(...)`: Processes profile updates. *Calls `Profile_Model::updateProfile`.*
    *   `handlePasswordUpdate(...)`: Processes password changes. *Calls `Profile_Model::updatePassword`.*

#### 5.1.6. `Salary_Control`
*   **Role:** Orchestrates the salary calculation and display for the current user.
*   **Variables:**
    *   `view` (Pointer to `Salary_View`).
    *   `model` (Pointer to `Salary_Model`).
    *   `currentSession` (Pointer to `SessionManager`).
*   **Functions:**
    *   `loadData(int month, int year)`: Loads salary data for the specified month/year. *Calls `Salary_Model::getSalarySummary`.*
    *   `onMonthYearChanged(int month, int year)`: Slot handling user's date selection.

#### 5.1.7. `Schedule_Control`
*   **Role:** Manages logic for creating and modifying schedules (used heavily by Managers).
*   **Variables:**
    *   `view` (Pointer to `Schedule_View`).
    *   `model` (Pointer to `Schedule_Model`).
    *   `currentEmployeeId` (ID of the employee being managed).
*   **Functions:**
    *   `load()`: Loads schedule data.
    *   `onAddShiftRequested(...)`: Handles adding a new shift. *Calls `Schedule_Model::handleAddShiftSubmission`.*
    *   `onSaveShiftRequested()`: Handles saving the schedule draft. *Calls `Schedule_Model::saveDraftShiftsToDatabase`.*

#### 5.1.8. `ViewSchedule_Control`
*   **Role:** Manages logic for viewing existing schedules.
*   **Variables:**
    *   `view` (Pointer to `ViewSchedule_View`).
    *   `model` (Pointer to `Schedule_Model`).
    *   `currentSession` (Pointer to `SessionManager`).
*   **Functions:**
    *   `loadData()`, `loadStaffSchedule()`, `loadManagerSchedule()`: Loads schedule based on role. *Calls `Schedule_Model::getAcceptedSchedule` or `getManagerWeeklyGrid`.*
    *   `onPrevWeek()`, `onNextWeek()`, `onCurrentWeek()`: Navigation slots.

### 5.2. Core Module (`src/core`)

This module defines the foundational entities and domain objects of the application.

#### 5.2.1. `User` (Abstract)
*   **Role:** Represents a generic person in the system.
*   **Variables:**
    *   `role`, `idEmployee`, `avatarPath`, `idCitizenIdentity`, `name`, `dob`, `address`, `phoneNum`, `gender`. *Used throughout the app to display or edit user info.*
*   **Functions:**
    *   Getters and Setters for all variables.
    *   `getSalary()`: Pure virtual function to be implemented by derived classes.

#### 5.2.2. `Staff` (Inherits `User`)
*   **Role:** Represents a staff-level employee.
*   **Variables:**
    *   `hourSalary`, `hourWork`.
*   **Functions:**
    *   `getSalary()`: Calculates salary based on hourly rate and hours worked. *Called by `Salary_Model`.*

#### 5.2.3. `Manager` (Inherits `User`)
*   **Role:** Represents a manager-level employee.
*   **Variables:**
    *   `fixSalary`, `dayWork`.
*   **Functions:**
    *   `getSalary()`: Calculates salary based on fixed rate and days worked. *Called by `Salary_Model`.*

#### 5.2.4. `UserFactory`
*   **Role:** Factory class to create instances of `User` (`Staff` or `Manager`).
*   **Functions:**
    *   `createContainsUser(...)`: Instantiates a user from DB records. *Called by `Employee_Model::loadData`.*
    *   `createNewUser(...)`: Creates a new user instance.

#### 5.2.5. `Shift`
*   **Role:** Represents a single work shift.
*   **Variables:**
    *   `date`, `EmployeeID`, `dayOfWeek`, `startTime`, `endTime`, `status`, `shiftId`.
*   **Functions:**
    *   Getters and setters for shift properties. *Called heavily by `Schedule_Model` and `Schedule_Control`.*

#### 5.2.6. `ShiftBlock`
*   **Role:** Represents a block of time in a schedule containing multiple assigned users.
*   **Variables:**
    *   `date`, `startTime`, `endTime`, `employees` (List of `User*`).
*   **Functions:**
    *   `addStaff(User* u)`: Assigns a user to this block.
    *   `getStatus()`: Determines if the block is empty, understaffed, or sufficient. *Calls `Config::getMinStaffPerShift`.*

#### 5.2.7. `Optimizer`
*   **Role:** Contains the algorithm (Max Flow / Min Cost Flow) for automatically generating schedules.
*   **Variables:**
    *   Graph variables (`m_edges`, `m_g`), `shifts`, `userMinutes`.
*   **Functions:**
    *   `solve()`: Executes the optimization algorithm. *Called by `Schedule_Model::generateSchedule`.*
    *   `minCostFlow(...)`: Core algorithmic function.

### 5.3. Model Module (`src/model`)

This module handles database queries, business rules, and state management.

#### 5.3.1. `Change_password`
*   **Role:** Handles the logic and validation for changing a user's password.
*   **Functions:**
    *   `updatePassword(...)`: Orchestrates the password update process. *Calls `verifyOldPassword`, `validatePasswordStrength`, `executePasswordUpdate`. Called by `Profile_Model`.*

#### 5.3.2. `Employee_Model`
*   **Role:** Manages the collection of employees and performs CRUD operations against the database.
*   **Variables:**
    *   `listEmployee` (List of `User*`).
*   **Functions:**
    *   `loadData()`: Loads all employees from DB. *Calls `Database::execQuery`.*
    *   `SearchSortFilter(...)`: Applies search, sort, and filter algorithms (e.g., Rabin-Karp) on the employee list.
    *   `addEmployee(...)`, `updateEmployee(...)`, `deleteEmployee(...)`: Modifies DB state.

#### 5.3.3. `Login_Model`
*   **Role:** Verifies credentials against the database.
*   **Functions:**
    *   `verifyLogin(userName, password)`: Hashes input and checks against DB. *Calls `Security::hashPassword` and `UserFactory::createContainsUser`.*

#### 5.3.4. `Profile_Model`
*   **Role:** Handles updating a specific user's profile data.
*   **Functions:**
    *   `updateProfile(...)`: Updates demographic info in DB.
    *   `updatePassword(...)`: *Delegates to `Change_password::updatePassword`.*

#### 5.3.5. `Salary_Model`
*   **Role:** Calculates and summarizes salary data based on shifts and roles.
*   **Functions:**
    *   `getSalarySummary(...)`, `getNormalDaysData(...)`: Aggregates shift data into salary metrics. *Calls `Database::execQuery` and `Shift` getters.*

#### 5.3.6. `Schedule_Model`
*   **Role:** Manages schedule data, drafts, and coordinates auto-generation.
*   **Variables:**
    *   `shiftList`, `draftShifts`, `currentWeeklyUsers`.
*   **Functions:**
    *   `checkOverlapping(...)`: Validates time overlaps for new shifts.
    *   `generateSchedule()`: Runs the auto-scheduler. *Calls `Optimizer::solve`.*
    *   `saveDraftShiftsToDatabase()`: Commits drafts to the DB.

#### 5.3.7. `Validator`
*   **Role:** Utility class providing static methods to validate common user inputs.
*   **Functions:**
    *   `isValidPassword()`, `isValidDate()`, `isValidPhoneNumber()`, `isValidCitizenId()`. *Called by Views/Dialogs (e.g., `AddEmployee_Dialog`) before submission.*

### 5.4. Utils Module (`src/utils`)

This module provides cross-cutting utilities like configuration, database connections, and session state.

#### 5.4.1. `Config`
*   **Role:** Stores global configuration constants (e.g., store opening hours).
*   **Variables:**
    *   `openHour`, `closeHour`, `minStaffPerShift`, etc.
*   **Functions:**
    *   Getters for constants. *Called by `Schedule_Model` and `ShiftBlock`.*

#### 5.4.2. `Database`
*   **Role:** Singleton class managing the SQLite database connection.
*   **Variables:**
    *   `instance`, `dbConnect`.
*   **Functions:**
    *   `getInstance()`: Retrieves the singleton.
    *   `execQuery(const QString& query)`: Executes a SQL statement. *Called by almost all Model classes.*

#### 5.4.3. `Security`
*   **Role:** Provides security-related functions like hashing.
*   **Functions:**
    *   `hashPassword(...)`: Hashes passwords using SHA-256. *Called by `Login_Model` and `Change_password`.*

#### 5.4.4. `SessionManager`
*   **Role:** Singleton class tracking the currently authenticated user and managing permissions (RBAC).
*   **Variables:**
    *   `currentUser` (Pointer to `User`).
*   **Functions:**
    *   `saveCurrentInfo(...)`, `getCurrentUser()`.
    *   `checkPermission(requiredRole)`: Validates if the current user has the required role. *Called by controllers to restrict access.*

### 5.5. View Module (`src/view`)

This module contains the UI components constructed using Qt (Widgets and Dialogs).

#### 5.5.1. Forms & Dialogs
*   **`AddEmployee_Dialog` / `EditEmployee_Dialog`**: UI forms for creating/editing employees. Collects input fields and provides delegates for validation. *Instantiated by `Employee_Control`.*
*   **`EditPassword_Widget` / `EditProfile_Widget`**: Sliding panels within the Profile page for updating credentials/info.

#### 5.5.2. Main Views
*   **`Login_View`**: Handles user credential input. Emits `loginSubmitted`. *Controller: `Login_Control`.*
*   **`Dashboard_View` / `Overview_View` / `Main_View`**: Landing pages providing a high-level summary.
*   **`Profile_View`**: Displays user info. *Controller: `Profile_Control`.*
*   **`Salary_View`**: Displays salary breakdowns in tables. *Controller: `Salary_Control`.*
*   **`Schedule_View` / `ViewSchedule_View`**: Interfaces for managers to create schedules and for all users to view them. Includes interactive schedule grids. *Controllers: `Schedule_Control`, `ViewSchedule_Control`.*

#### 5.5.3. Custom Widgets
*   **`EmployeesWidget`**: A complex view containing the employee table, filter dropdowns, and pagination. Emits complex signals for search/sort/filter. *Controller: `Employee_Control`.*
*   **`EmployeeCard`**: A small widget to display a summary of a single employee in grid layouts.
*   **`Sidebar_Widget`**: The navigation menu. Highlights active tabs and handles navigation clicks.
*   **`View_Navigator`**: The main application window containing a `QStackedWidget` to swap between different main views. *Controller: `Control_Navigator`.*
