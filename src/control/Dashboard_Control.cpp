#include "global.h"
#include "Dashboard_Control.h"
#include "view/Dashboard_View.h"
#include "view/employeecard.h"
#include <QTimer>

// ==============================================================================
// LOGIC FLOW:
// 1. Dashboard_Control is constructed, setting up timers for auto-refresh.
// 2. setView() is called to bind the Dashboard_View instance and its signals.
// 3. init() is called to load initial data:
//    a. Employee_Model::loadData() to fetch all employees.
//    b. loadEmployeeCards() -> Dashboard_Model::getWorkingEmployeeIds() to show active staff.
//    c. loadShiftPanel() -> Dashboard_Model::getNextShiftEmployees() to show next shift staff.
//    d. loadSalaryChart() -> Dashboard_Model::getSalaryStats() to display the chart.
// 4. Timer automatically calls autoRefresh() every hour to update the data.
// 5. User interactions in view trigger onYearChanged() or profilePageClicked().
// ==============================================================================

Dashboard_Control::Dashboard_Control(QObject *parent)
    : QObject(parent)
    , view(nullptr)
    , currentSession(SessionManager::getInstance())
    , empModel(new Employee_Model())
    , dashModel(new Dashboard_Model())
    , m_refreshTimer(new QTimer(this))
    , m_selectedYear(QDate::currentDate().year())
{
    m_refreshTimer->setSingleShot(true);
    connect(m_refreshTimer, &QTimer::timeout, this, &Dashboard_Control::autoRefresh);
    scheduleNextHourRefresh();
}

Dashboard_Control::~Dashboard_Control()
{
    delete empModel;
    delete dashModel;
}

// Initializes the dashboard with data.
// Called by: Main application/Navigator when switching to Dashboard
// Calls: Employee_Model::loadData(), Employee_Model::getListEmployee(), loadEmployeeCards(), loadShiftPanel(), loadSalaryChart()
void Dashboard_Control::init()
{
    if (!view) return;

    dashModel->clearCache(); // Clear stats cache when reloading the dashboard
    empModel->loadData();
    QList<User*> all = empModel->getListEmployee();

    loadEmployeeCards(all);
    loadShiftPanel();
    loadSalaryChart();
    loadLeaveRequestPanel();
}

Dashboard_View* Dashboard_Control::getView() { return view; }

// Binds the view to this controller and connects signals.
// Called by: Main application/Navigator during setup
// Calls: QObject::connect()
void Dashboard_Control::setView(Dashboard_View* v)
{
    view = v;
    if (!view) return;
    QObject::connect(view, &Dashboard_View::profileClicked,
                     this, &Dashboard_Control::profilePageClicked);
    QObject::connect(view, &Dashboard_View::yearChanged,
                     this, &Dashboard_Control::onYearChanged);
    QObject::connect(view, &Dashboard_View::leaveRequestReviewRequested,
                     this, &Dashboard_Control::reviewLeaveRequest);
}

// Handles year tab changes from the view.
// Called by: Dashboard_View (via yearChanged signal)
// Calls: loadSalaryChart()
void Dashboard_Control::onYearChanged(int year)
{
    m_selectedYear = year;
    loadSalaryChart();
}

// Calculates time until the next hour and schedules a refresh.
// Called by: Constructor, autoRefresh()
// Calls: QTime::currentTime(), QTimer::start()
void Dashboard_Control::scheduleNextHourRefresh()
{
    QTime now = QTime::currentTime();
    int msToNextHour = (59 - now.minute()) * 60000 + (59 - now.second()) * 1000 + (1000 - now.msec());
    // Adding 1000ms extra padding to make sure it crosses the hour boundary
    m_refreshTimer->start(msToNextHour + 1000);
}

// Automatically refreshes dashboard data periodically.
// Called by: QTimer (timeout signal)
// Calls: Employee_Model::getListEmployee(), loadEmployeeCards(), loadShiftPanel(), scheduleNextHourRefresh()
void Dashboard_Control::autoRefresh()
{
    if (!view) return;
    QList<User*> all = empModel->getListEmployee();
    loadEmployeeCards(all);
    loadShiftPanel();
    loadLeaveRequestPanel();

    // Re-schedule for the next hour
    scheduleNextHourRefresh();
}

// Populates the current shift employee cards in the view.
// Called by: init(), autoRefresh()
// Calls: Dashboard_View::clearEmployeeGrid(), Dashboard_Model::getWorkingEmployeeIds(), EmployeeCard::setData(), Dashboard_View::addEmployeeCard()
void Dashboard_Control::loadEmployeeCards(const QList<User*>& list)
{
    view->clearEmployeeGrid();

    QSet<int> workingIds = dashModel->getWorkingEmployeeIds();

    for (User* u : list) {
        if (!u) continue;
        if (!workingIds.contains(u->getIdEmployee())) continue;

        EmployeeCard* card = new EmployeeCard();
        card->setData(
            u->getAvatarPath(),
            u->getName(),
            u->getRole(),
            u->getPhoneNum(),
            QString::number(u->getIdEmployee()),
            u->getDOB(),
            u->getGender()
        );
        card->setStatus(true);
        view->addEmployeeCard(card);
    }
}

// Populates the next shift panel in the view.
// Called by: init(), autoRefresh()
// Calls: Dashboard_Model::getNextShiftEmployees(), Dashboard_View::updateNextShiftPanel()
void Dashboard_Control::loadShiftPanel()
{
    QList<ShiftEmployeeInfo> nextShift = dashModel->getNextShiftEmployees();
    view->updateNextShiftPanel(nextShift);
}

// Populates the salary chart in the view based on permissions.
// Called by: init(), onYearChanged()
// Calls: SessionManager::checkPermission(), Dashboard_View::setSalaryChartVisible(), Dashboard_Model::getSalaryStats(), Dashboard_View::updateSalaryChart()
void Dashboard_Control::loadSalaryChart()
{
    bool hasPermission = currentSession->checkPermission("Manager") || currentSession->checkPermission("Admin");
    view->setSalaryChartVisible(hasPermission);

    if (hasPermission) {
        SalaryChartData data = dashModel->getSalaryStats(m_selectedYear);
        view->updateSalaryChart(
            data.lastYearMonthly,
            data.thisYearMonthly,
            data.lastYearEmpCount,
            data.thisYearEmpCount,
            m_selectedYear
        );
    }
}

void Dashboard_Control::loadLeaveRequestPanel()
{
    const bool managerMode = currentSession &&
        (currentSession->checkPermission("Manager") || currentSession->checkPermission("Admin"));
    view->setLeaveRequestPanelVisible(managerMode);
    if (managerMode)
        view->updateLeaveRequestPanel(leaveRequestModel.getPendingLeaveRequests());
}

void Dashboard_Control::reviewLeaveRequest(int requestId, bool approved)
{
    if (!view || !currentSession || !currentSession->getCurrentUser())
        return;
    User *user = currentSession->getCurrentUser();
    if (user->getRole() != "Manager" && user->getRole() != "Admin")
        return;

    const auto choice = QMessageBox::question(
        view, QString::fromUtf8("Xử lý yêu cầu nghỉ phép"),
        approved ? QString::fromUtf8("Duyệt yêu cầu nghỉ phép này?")
                 : QString::fromUtf8("Từ chối yêu cầu nghỉ phép này?"),
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);
    if (choice == QMessageBox::Cancel)
        return;

    bool accepted = false;
    const QString reason = QInputDialog::getText(
        view, QString::fromUtf8("Lý do quyết định"),
        approved
            ? QString::fromUtf8("Ghi chú phê duyệt (không bắt buộc):")
            : QString::fromUtf8("Lý do từ chối:"),
        QLineEdit::Normal, {}, &accepted).trimmed();
    if (!accepted)
        return;

    QString error;
    if (!leaveRequestModel.decideLeaveRequest(requestId, user->getIdEmployee(),
                                               approved, reason, &error)) {
        QMessageBox::warning(view, QString::fromUtf8("Không thể xử lý"), error);
        loadLeaveRequestPanel();
        return;
    }
    notificationModel.markLeaveRequestReviewedByRequest(requestId, approved);
    loadLeaveRequestPanel();
    emit leaveRequestDecisionCompleted();
}
