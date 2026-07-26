#include "Salary_Model.h"


Salary_Model::Salary_Model(QObject *parent) : QObject(parent)
{
}

Salary_Model::~Salary_Model()
{
}

SalaryData Salary_Model::getSalarySummary(User* user, int month, int year)
{
    SalaryData data = {0, 0, 0, 0, 0, 0};
    QMap<QString, int> normalDays = getNormalDaysData(user, month, year);
    QMap<QString, int> holidayDays = getHolidayDaysData(user, month, year);

    if (user->getRole() == "Manager") {
        int totalDaysWorked = normalDays.size() + holidayDays.size();
        QDate date(year, month, 1);
        int absentDays = date.daysInMonth() - totalDaysWorked;
        if (absentDays < 0) absentDays = 0;
        
        data.normalHours = normalDays.size();
        data.holidayHours = holidayDays.size();
        data.normalSalary = data.normalHours * user->getSalary();
        data.holidaySalary = data.holidayHours * (user->getSalary() * 2);
        data.penalty = absentDays * 2000000;

        data.totalSalary = data.normalSalary + data.holidaySalary - data.penalty;
    } else if (user->getRole() == "Staff") {
        for (int hours : normalDays.values()) {
            data.normalHours += hours;
        }
        for (int hours : holidayDays.values()) {
            data.holidayHours += hours;
        }
        data.normalSalary = data.normalHours * user->getSalary();
        data.holidaySalary = data.holidayHours * (user->getSalary() * 2);
        data.penalty = 50000;
        data.totalSalary = data.normalSalary + data.holidaySalary - data.penalty;
    }
    return data;
}

QMap<QString, int> Salary_Model::getNormalDaysData(User* user, int month, int year)
{
    QMap<QString, int> data;

    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);

    QDate startDate(year, month, 1);
    QDate endDate = QDate(year, month + 1, 1).addDays(-1);
    if (endDate > QDate::currentDate()) endDate = QDate::currentDate();

    //qDebug() << "id: " << user->getIdEmployee();

    query.prepare("SELECT * FROM SHIFT WHERE idEmployee = :id "
                  "AND workDate BETWEEN :start AND :end AND status = 1 AND isHoliday = 0");
    query.bindValue(":id", user->getIdEmployee());
    query.bindValue(":start", startDate);
    query.bindValue(":end", endDate);

    if (!query.exec())
        return data;
    while (query.next())
    {
        QString date = query.value("workDate").toString();
        if (data.find(date) == data.end())  data[date] = 0;
        data[date] += query.value("startTime").toTime().secsTo(query.value("endTime").toTime()) / 3600;

    }
    return data;
}

QMap<QString, int> Salary_Model::getHolidayDaysData(User* user, int month, int year)
{
    QMap<QString, int> data;

    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);

    QDate startDate(year, month, 1);
    QDate endDate = QDate(year, month + 1, 1).addDays(-1);
    if (endDate > QDate::currentDate()) endDate = QDate::currentDate();

    //qDebug() << "id: " << user->getIdEmployee();

    query.prepare("SELECT * FROM SHIFT WHERE idEmployee = :id "
                  "AND workDate BETWEEN :start AND :end AND status = 1 AND isHoliday = 1");
    query.bindValue(":id", user->getIdEmployee());
    query.bindValue(":start", startDate);
    query.bindValue(":end", endDate);

    if (!query.exec())
        return data;
    while (query.next())
    {
        QString date = query.value("workDate").toString();
        if (data.find(date) == data.end())  data[date] = 0;
        data[date] += query.value("startTime").toTime().secsTo(query.value("endTime").toTime()) / 3600;

    }
    return data;
}

