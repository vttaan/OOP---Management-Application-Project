#include "Salary_Control.h"
#include <QDate>

Salary_Control::Salary_Control(QObject *parent)
    : QObject(parent), currentSession(SessionManager::getInstance())
{
    model = new Salary_Model(this);
}

Salary_Control::~Salary_Control()
{
}

void Salary_Control::init(){
    QDate current = QDate::currentDate();
    loadData(current.month(), current.year());
}

void Salary_Control::setView(Salary_View* view)
{
    this->view = view;
    
    // Connect view signals to control slots if needed
    connect(this->view, &Salary_View::monthYearChanged, this, &Salary_Control::onMonthYearChanged);
}

Salary_View* Salary_Control::getView() const
{
    return view;
}

void Salary_Control::loadData(int month, int year)
{
    if(!view) return;
    
    // Fetch data from model
    User* currentUser = currentSession->getCurrentUser();
    SalaryData summary = model->getSalarySummary(currentUser, month, year);
    QMap<QString, int> normalDays = model->getNormalDaysData(currentUser, month, year);
    QMap<QString, int> holidayDays = model->getHolidayDaysData(currentUser, month, year);
    QString baseSalary = QString::number(currentUser->getSalary());
    
    // Update view
    view->setRole(currentUser->getRole());
    view->setBaseSalary(baseSalary);
    view->populateNormalTable(normalDays);
    view->populateHolidayTable(holidayDays);
    view->populateSummaryTable(summary);
}

void Salary_Control::onMonthYearChanged(int month, int year)
{
    loadData(month, year);
}
