#include "global.h"
#include "Dashboard_Control.h"
#include "view/Dashboard_View.h"
#include "view/employeecard.h"
#include <QTimer>

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

void Dashboard_Control::init()
{
    if (!view) return;

    empModel->loadData();
    QList<User*> all = empModel->getListEmployee();

    loadEmployeeCards(all);
    loadShiftPanel();
    loadSalaryChart();
}

Dashboard_View* Dashboard_Control::getView() { return view; }

void Dashboard_Control::setView(Dashboard_View* v)
{
    view = v;
    if (!view) return;
    QObject::connect(view, &Dashboard_View::profileClicked,
                     this, &Dashboard_Control::profilePageClicked);
    QObject::connect(view, &Dashboard_View::yearChanged,
                     this, &Dashboard_Control::onYearChanged);
}

// Reload only the chart when user selects a different year tab
void Dashboard_Control::onYearChanged(int year)
{
    m_selectedYear = year;
    loadSalaryChart();
}

void Dashboard_Control::scheduleNextHourRefresh()
{
    QTime now = QTime::currentTime();
    int msToNextHour = (59 - now.minute()) * 60000 + (59 - now.second()) * 1000 + (1000 - now.msec());
    // Adding 1000ms extra padding to make sure it crosses the hour boundary
    m_refreshTimer->start(msToNextHour + 1000);
}

void Dashboard_Control::autoRefresh()
{
    if (!view) return;
    QList<User*> all = empModel->getListEmployee();
    loadEmployeeCards(all);
    loadShiftPanel();

    // Re-schedule for the next hour
    scheduleNextHourRefresh();
}

// Panel 1: Employee cards for the current active shift
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

// Panel 2 (next shift)
void Dashboard_Control::loadShiftPanel()
{
    QList<ShiftEmployeeInfo> nextShift = dashModel->getNextShiftEmployees();
    view->updateNextShiftPanel(nextShift);
}

// Panel 3: Salary bar chart for (m_selectedYear - 1) vs m_selectedYear
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