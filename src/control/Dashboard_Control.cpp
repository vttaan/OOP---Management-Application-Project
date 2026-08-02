#include "global.h"
#include "Dashboard_Control.h"
#include "view/Dashboard_View.h"
#include "view/employeecard.h"

Dashboard_Control::Dashboard_Control(QObject *parent)
    : QObject(parent)
    , view(nullptr)
    , currentSession(SessionManager::getInstance())
    , empModel(new Employee_Model())
    , dashModel(new Dashboard_Model())
    , m_selectedYear(QDate::currentDate().year())
{}

Dashboard_Control::~Dashboard_Control()
{
    delete empModel;
    delete dashModel;
}

void Dashboard_Control::init()
{
    if (!view) return;

    // Seed sample data so the dashboard always shows something on first run
    dashModel->seedTodayShifts();

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

// Panel 1: Employee cards for the current active shift
void Dashboard_Control::loadEmployeeCards(const QList<User*>& list)
{
    view->clearEmployeeGrid();

    QSet<int> workingIds = dashModel->getWorkingEmployeeIds();

    for (User* u : list) {
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

// Panel 2 (next shift) + Panel 4 (absent employees)
void Dashboard_Control::loadShiftPanel()
{
    QList<ShiftEmployeeInfo> nextShift = dashModel->getNextShiftEmployees();
    view->updateNextShiftPanel(nextShift);

    QStringList absent = dashModel->getAbsentEmployees();
    view->updateAbsentPanel(absent);
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