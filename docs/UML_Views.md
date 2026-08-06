# UML View Layer Diagram — Optimus Management Application

> Đoạn code dưới đây chứa toàn bộ các class trong thư mục `src/view`, bao gồm đầy đủ thuộc tính (attributes), phương thức (methods) và các mối quan hệ (relationships). 
> **Cách dùng:** Copy phần trong block `mermaid` và chèn vào Draw.io (Insert -> Advanced -> Mermaid...)

```mermaid
classDiagram

    %% =======================
    %% MASTER NAVIGATOR & MAIN
    %% =======================
    class View_Navigator {
        -currentWindow : QWidget*
        -controller : Control_Navigator*
        +loginPage : Login_View*
        +dashboardPage : Dashboard_View*
        +profilePage : Profile_View*
        +employeePage : Employee_View*
        +schedulePage : Schedule_View*
        +salaryPage : Salary_View*
        +sidebar : Sidebar_Widget*
        +viewSchedulePage : ViewSchedule_View*
        +settingPage : Setting_View*
        +notificationPage : Notification_View*
        +ui : Ui::View_Navigator*
        +View_Navigator(controller, parent)
        +getSideBar() Sidebar_Widget*
        +getController() Control_Navigator*
        +getWindow() QWidget*
        +getUI() Ui::View_Navigator*
        +setPageIndex(index : int)
        ~logoutSubmitted()
    }

    class Main_View {
        -ui : Ui::Overview_view*
        -gridLayoutEmployees : QGridLayout*
        -currentRow : int
        -currentCol : int
        +Main_View(parent)
        +getSidebar() QWidget*
        +switchPage(pageIndex : int)
        +clearEmployeeCards()
        +addEmployeeCard(avatarPath, name, role, email, phone)
        +clearSidePanels()
        +addNextShiftItem(name, timeInfo, colorHex)
        +addOffEmployeeItem(name, reason, colorHex)
        #eventFilter(watched, event) bool
        ~menuOverviewClicked()
        ~menuHRClicked()
        ~menuTimekeepClicked()
        ~menuSalaryClicked()
        ~menuReportClicked()
        ~menuSettingsClicked()
        ~profileClicked()
        ~toggleSidebarClicked()
    }

    class Sidebar_Widget {
        -ui : Ui::Sidebar_Widget*
        -normalStyle : QString
        -logOutStyle : QString
        -activeStyle : QString
        +Sidebar_Widget(parent)
        +getNormalStyle() QString
        +getActiveStyle() QString
        +loadUserData(session : SessionManager*)
        +setPermission(permitted : bool)
        +updateButtonStyles(mainIndex : int)
        +setNotificationCount(count : int)
        +hideSubMenuInSchedule()
        #paintEvent(event)
        -setupSidebarAvatar(imagePath : QString)
        -initUI()
        ~menuClicked(pageIndex : int)
        ~logoutClicked()
    }

    %% =======================
    %% AUTH & DASHBOARD
    %% =======================
    class Login_View {
        -ui : Ui::Login_View*
        -hidePassword : QAction*
        -bgPixmap : QPixmap
        -controller : Login_Control*
        +Login_View(controller, parent)
        +clearInputs()
        +clearPassword()
        +getController() Login_Control*
        +setController(controller)
        -setupUI()
        -togglePassword()
        -initSignals()
        -paintEvent(event)
        -on_btnLogin_clicked()
        -on_txtLoginPassword_returnPressed()
        ~loginSubmitted(username, password)
        ~loginSuccessful()
    }

    class Dashboard_View {
        -ui : Ui::Dashboard_View*
        -controller : Dashboard_Control*
        -m_nextShiftLayout : QVBoxLayout*
        -m_leaveRequestLayout : QVBoxLayout*
        -m_lblLastYearCount : QLabel*
        -m_lblThisYearCount : QLabel*
        -m_tooltipLabel : QLabel*
        -m_chart : QChart*
        -m_chartView : QChartView*
        -m_salaryCard : QFrame*
        -m_leaveRequestCard : QFrame*
        -m_empGridWidget : QWidget*
        -m_yearTabBar : QTabBar*
        -m_availableYears : QVector~int~
        +Dashboard_View(controller, parent)
        +clearEmployeeGrid()
        +addEmployeeCard(card : EmployeeCard*)
        +updateNextShiftPanel(entries : QList~ShiftEmployeeInfo~)
        +updateLeaveRequestPanel(requests : QList~LeaveRequestInfo~)
        +setLeaveRequestPanelVisible(visible : bool)
        +updateSalaryChart(lastYear, thisYear, lastCount, thisCount, year)
        +setSalaryChartVisible(visible : bool)
        -onYearTabClicked(index : int)
        -onBarHovered(status, index, barSet)
        -makeCard(title, innerLayout, isDark) QFrame*
        ~profileClicked()
        ~yearChanged(year : int)
        ~leaveRequestReviewRequested(requestId, approved)
    }

    class EmployeeCard {
        -ui : Ui::EmployeeCard*
        +EmployeeCard(parent)
        +setData(avatarPath, name, role, phone, id, dob, gender)
        +setStatus(isWorking : bool)
    }

    %% =======================
    %% PROFILE
    %% =======================
    class Profile_View {
        -ui : Ui::Profile_View*
        -controller : Profile_Control*
        -editProfileWidget : EditProfile_Widget*
        -editPasswordWidget : EditPassword_Widget*
        +Profile_View(controller, parent)
        +getController() Profile_Control*
        +setController(controller)
        +loadUserData(currentSession : SessionManager*)
        +loadUserData(name, studentId, dob, phone, email, avatarPath)
        -setupAvatar(imagePath : QString)
        #resizeEvent(event)
        -on_backButton_clicked()
        -on_btnEditInfo_clicked()
        -on_btnEditPassword_clicked()
    }

    class EditProfile_Widget {
        -panelWidget : QWidget*
        -animation : QPropertyAnimation*
        -txtName : QLineEdit*
        -txtDob : QLineEdit*
        -txtAddress : QLineEdit*
        -txtPhone : QLineEdit*
        -txtCitizenId : QLineEdit*
        -cmbGender : QComboBox*
        -lblAvatarPreview : QLabel*
        -currentAvatarPath : QString
        -isPanelOpen : bool
        +EditProfile_Widget(parent)
        +setInitialData(name, dob, address, phone, citizenId, avatarPath, gender)
        +slideIn()
        +slideOut()
        #paintEvent(event)
        #mousePressEvent(event)
        #resizeEvent(event)
        -onSaveClicked()
        -onCancelClicked()
        -onChangeAvatarClicked()
        -onAnimationFinished()
        ~saveRequested(name, dob, address, phone, citizenId, avatarPath, gender)
    }

    class password_LineEdit {
        +toggleIcon : QAction*
        +password_LineEdit(parent)
        +setup()
        +togglePasword()
    }

    class EditPassword_Widget {
        -panelWidget : QWidget*
        -animation : QPropertyAnimation*
        -lblAvatarPreview : QLabel*
        -currentAvatarPath : QString
        -isPanelOpen : bool
        +txtOldPassword : password_LineEdit*
        +txtNewPassword : password_LineEdit*
        +txtConfirmPassword : password_LineEdit*
        +EditPassword_Widget(parent)
        +setInitialData()
        +slideIn()
        +slideOut()
        #paintEvent(event)
        #mousePressEvent(event)
        #resizeEvent(event)
        -onSaveClicked()
        -onCancelClicked()
        -onAnimationFinished()
        ~saveRequested(oldPassword, newPassword)
    }

    %% =======================
    %% EMPLOYEE MANAGEMENT
    %% =======================
    class Employee_View {
        -ui : Ui::Employee_View*
        -m_allEmployees : QList~User*~
        -filterDropdown : QFrame*
        -chkCashier : QCheckBox*
        -chkHallStaff : QCheckBox*
        -chkKitchenAssistant : QCheckBox*
        -chkManager : QCheckBox*
        -chkAdmin : QCheckBox*
        -chkMale : QCheckBox*
        -chkFemale : QCheckBox*
        -m_filterOpen : bool
        -sortDropdown : QFrame*
        -m_sortOpen : bool
        -m_sortField : QString
        -m_sortDir : int
        +Employee_View(parent)
        +loadEmployees(employees : QList~User*~)
        +showError(msg : QString)
        +showSuccess(msg : QString)
        -handleAddEmployee()
        -emitUpdateRequest()
        -toggleFilterDropdown()
        -toggleSortDropdown()
        -setupTableHeader()
        -setupConnections()
        -buildFilterDropdown()
        -buildSortDropdown()
        -renderTable(employees)
        -createAvatar(avatarPath) QLabel*
        -createRoleBadge(role) QLabel*
        -createPayTypeBadge(payType) QLabel*
        -createActionButton(iconPath, tooltip) QPushButton*
        ~backToDashboard()
        ~requestAddEmployee()
        ~requestEditEmployee(idEmployee)
        ~requestDeleteEmployee(idEmployee)
        ~requestUpdate(search, filter, sort, dir)
    }

    class AddEmployee_Dialog {
        -m_avatarPath : QString
        -inpName : QLineEdit*
        -inpPhone : QLineEdit*
        -inpDob : QDateEdit*
        -inpAddress : QLineEdit*
        -inpCitizenId : QLineEdit*
        -inpUsername : QLineEdit*
        -inpPassword : QLineEdit*
        -inpSalary : QLineEdit*
        -cmbRole : QComboBox*
        -cmbGender : QComboBox*
        -cmbIsFixedSalary : QComboBox*
        -lblAvatarPreview : QLabel*
        -btnUpload : QPushButton*
        -btnConfirm : QPushButton*
        -btnCancel : QPushButton*
        -lblError : QLabel*
        +validatorDelegate : function
        +passwordGeneratorDelegate : function
        +usernameGeneratorDelegate : function
        +AddEmployee_Dialog(parent)
        +getName() QString
        +getRole() QString
        +getGender() QString
        +getPhone() QString
        +getDob() QString
        +getAddress() QString
        +getCitizenId() QString
        +getAvatarPath() QString
        +getSalary() int
        +getIsFixedSalary() bool
        +isDobSelected() bool
        +getUsername() QString
        +getPassword() QString
        -setupUi()
        -validate() bool
        -onConfirm()
    }

    class EditEmployee_Dialog {
        -m_avatarPath : QString
        -inpName : QLineEdit*
        -inpPhone : QLineEdit*
        -inpDob : QDateEdit*
        -inpAddress : QLineEdit*
        -inpCitizenId : QLineEdit*
        -inpSalary : QLineEdit*
        -cmbRole : QComboBox*
        -cmbGender : QComboBox*
        -cmbIsFixedSalary : QComboBox*
        -cmbStatus : QComboBox*
        -lblAvatarPreview : QLabel*
        -btnUpload : QPushButton*
        -btnConfirm : QPushButton*
        -btnCancel : QPushButton*
        -lblError : QLabel*
        +validatorDelegate : function
        +EditEmployee_Dialog(emp : User*, parent)
        +getName() QString
        +getRole() QString
        +getPhone() QString
        +getDob() QString
        +getAddress() QString
        +getCitizenId() QString
        +getAvatarPath() QString
        +getGender() QString
        +getSalary() int
        +getIsFixedSalary() bool
        +getStatus() QString
        +isDobSelected() bool
        -setupUi(emp)
        -validate() bool
        -onConfirm()
    }

    class EmployeeDetails_Dialog {
        -dragPosition : QPoint
        +EmployeeDetails_Dialog(emp : User*, parent)
        #mousePressEvent(event)
        #mouseMoveEvent(event)
        -setupUi(emp)
        -getVietnameseRole(roleName) QString
        -getRoundedAvatar(avatarPath, name, role, size) QPixmap
        -addRowToForm(form, labelText, valueText, isHighlight)
    }

    %% =======================
    %% SCHEDULE & VIEWSCHEDULE
    %% =======================
    class Schedule_View {
        -ui : Ui::Schedule_View*
        -missingStaffWidget : QWidget*
        -lblMissingStaffHeader : QLabel*
        -lblMissingCount : QLabel*
        -tableMissingStaff : QTableWidget*
        -lblManagerDraftStatus : QLabel*
        -lblManagerSummary : QLabel*
        -managerStatusFilter : QComboBox*
        -managerRoleFilter : QComboBox*
        -managerUndoDraftButton : QPushButton*
        -managerClearDraftButton : QPushButton*
        -requestLeaveButton : QPushButton*
        -leaveHistoryButton : QPushButton*
        -shiftDetailDrawer : QFrame*
        -activeManagerAddButton : QPushButton*
        -m_managerAssignmentOpen : bool
        -m_isAssignMode : int
        -m_openHour : int
        -m_closeHour : int
        -m_isFullTimeMode : bool
        -m_partTimeRegistrationOpen : bool
        -m_partTimeDragActive : bool
        -m_fullTimeStatuses : FullTimeScheduleGrid
        +Schedule_View(parent)
        +enableRegistration(isEnable)
        +setPartTimeRegistrationState(isOpen, nextOpenDate)
        +showError(mess)
        +setUpInteractiveGrid(weekStart, openTime, closeTime)
        +setUpFullTimeScheduleGrid(weekStart, statuses)
        +showFullTimeSaveFeedback(message)
        +updateTableHeaders(monday)
        +updateStaffInteractiveGridStatus(pending, accepted, managerGrid)
        +showSuccess(msg)
        +showWarnings(warnings)
        +setManagerMode(isManager)
        +setManagerAssignmentState(isOpen, nextOpenDate)
        +updateManagerPendingGrid(grid)
        +updateAssignGrid(counts)
        +updateManagerMissingShifts(missingList)
        +setManagerDraftStatus(changeCount)
        +resetManagerAddButton()
        +updateManagerSummary(...)
        +updateManagerWeek(monday)
        +selectManagerShift(dayColumn, shiftRow)
        +showShiftRequestsDialog(requests, shiftLabel, eligible, date, start, end)
        -setUpUI()
        -buildInteractiveGrid(openHour, closeHour)
        -buildFullTimeGrid()
        -renderFullTimeCell(row, col)
        -updateFullTimeWeekMetadata(weekStart)
        -buttonSaveClicked()
        -clearPendingSelections()
        -onFullTimeCellClicked(row, col)
        ~requestSaveGridShifts(selectedHoursByDay)
        ~requestSaveFullTimeSchedule(selectedShiftsByDay)
        ~profileClicked()
        ~requestGenSchedule()
        ~requestConfirm()
        ~shiftBlockClicked(col, row)
        ~requestApproveShift(request)
        ~requestDeclineShift(request)
        ~requestAddEmployees(...)
        ~requestRemoveAssignedShift(...)
    }

    class ManagerEmployeeChooser_Dialog {
        -m_employees : QList~EligibleEmployeeInfo~
        -m_states : QMap~int, SelectionState~
        -m_blockStart : QTime
        -m_blockEnd : QTime
        -m_search : QLineEdit*
        -m_roleFilter : QComboBox*
        -m_sort : QComboBox*
        -m_table : QTableWidget*
        -m_selectedCount : QLabel*
        -m_confirmButton : QPushButton*
        +ManagerEmployeeChooser_Dialog(employees, blockStart, blockEnd, initialSelections, parent)
        +selections() QList~ManagerEmployeeSelection~
        -rebuildTable()
        -acceptSelection()
        -displayRole(role) QString
        -updateConfirmState()
    }

    class ViewSchedule_View {
        -ui : Ui::ViewSchedule_View*
        -detailsWidget : QWidget*
        -lblShiftDetailTitle : QLabel*
        -lblShiftDetailSubtitle : QLabel*
        -lblShiftDetailCount : QLabel*
        -tableShiftDetails : QTableWidget*
        -managerViewMode : QComboBox*
        -managerRoleFilter : QComboBox*
        -m_managerGrid : QMap
        -m_isManagerMode : bool
        +ViewSchedule_View(parent)
        +updateTable(weeklyData)
        +updatePendingTable(weeklyData)
        +updateManagerTable(gridData)
        +updateDateRange(monday)
        +highlightToday(currentDayIndex)
        +updateShiftDetails(employees, shiftIds, timeLabel)
        +setManagerFeaturesVisible(visible)
        +showReplacementDialog(oldShiftId, replacements)
        -setUpUI()
        -onBtnPrevClicked()
        -onBtnNextClicked()
        -onBtnCurrentClicked()
        ~requestPrevWeek()
        ~requestNextWeek()
        ~requestCurrentWeek()
        ~shiftClicked(row, dayIndex)
        ~requestShowReplacements(shiftId, role)
        ~requestConfirmReplacement(oldId, newId)
    }

    %% =======================
    %% OTHERS (SALARY, SETTING, NOTIFICATION)
    %% =======================
    class Salary_View {
        -ui : Ui::Salary_View*
        -isFixedEmployee : bool
        +Salary_View(parent)
        +setBaseSalary(salary : QString)
        +setEmployeeType(isFixed : bool)
        +populateNormalTable(data : QMap)
        +populateHolidayTable(data : QMap)
        +populateSummaryTable(data : SalaryData)
        -setupUI()
        -setupConnections()
        -showDetailDialog(date, hours, type)
        ~monthYearChanged(month, year)
    }

    class Setting_View {
        -ui : Ui::Setting_View*
        +Setting_View(parent)
        +loadData(openHour, closeHour, dayOpenRegis, roles, maxLeaveFT, maxDaysPT, maxHourPT)
        -saveClicked()
        -setupComboBox()
        -setupSpinBox()
        -setupTable()
        -initUI()
        ~requestSave(...)
        ~requestCancel()
    }

    class Notification_View {
        -filterBox : QComboBox*
        -markAllReadButton : QPushButton*
        -deleteReadButton : QPushButton*
        -emptyState : QLabel*
        -notificationScroll : QScrollArea*
        -notificationList : QWidget*
        -notificationListLayout : QVBoxLayout*
        -managerMode : bool
        +Notification_View(parent)
        +setNotifications(notifications, managerMode)
        +currentFilter() QString
        ~filterChanged(filter)
        ~markReadRequested(notificationId)
        ~markAllReadRequested()
        ~deleteReadRequested()
        ~reviewLeaveRequested(notificationId, leaveRequestId)
        ~openManagerScheduleRequested(notificationId)
    }

    %% =======================
    %% RELATIONSHIPS
    %% =======================
    View_Navigator *-- Login_View
    View_Navigator *-- Dashboard_View
    View_Navigator *-- Profile_View
    View_Navigator *-- Employee_View
    View_Navigator *-- Schedule_View
    View_Navigator *-- Salary_View
    View_Navigator *-- ViewSchedule_View
    View_Navigator *-- Setting_View
    View_Navigator *-- Notification_View
    View_Navigator *-- Sidebar_Widget

    Profile_View *-- EditProfile_Widget
    Profile_View *-- EditPassword_Widget
    EditPassword_Widget *-- password_LineEdit

    Employee_View ..> AddEmployee_Dialog : spawns
    Employee_View ..> EditEmployee_Dialog : spawns
    Employee_View ..> EmployeeDetails_Dialog : spawns

    Dashboard_View ..> EmployeeCard : uses
    Schedule_View ..> ManagerEmployeeChooser_Dialog : spawns

```
