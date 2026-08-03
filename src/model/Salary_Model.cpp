#include "Salary_Model.h"

const int PENALTY = 500000;


Salary_Model::Salary_Model(QObject *parent) : QObject(parent)
{
}

Salary_Model::~Salary_Model()
{
}

SalaryData Salary_Model::getSalarySummary(short int id, QString role, double base, int month, int year)
{
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);

    query.prepare("SELECT * FROM PROFILES WHERE idEmployee = :id ");
    query.bindValue(":id", id);
    query.exec();
    bool isFixedEmployee = query.value("isFixed").toBool();

    SalaryData data = {0, 0, 0, 0, 0, 0}; // normal hours, holiday hours, normal salary, holiday salary, penalty, total
    QMap<QString, int> normalDays = Salary_Model::getNormalDaysData(id, role, base, month, year);
    QMap<QString, int> holidayDays = Salary_Model::getHolidayDaysData(id, role, base, month, year);

    QDate startDate(year, month, 1);
    QDate endDate = startDate.addMonths(1).addDays(-1);
    if (endDate > QDate::currentDate()) endDate = QDate::currentDate();

    if (role == "Manager" || isFixedEmployee) {
        int totalDaysWorked = normalDays.size() + holidayDays.size();
        int absentDays = 0;
        if (QDate::currentDate() >= startDate) {
            absentDays = endDate.day() - totalDaysWorked;
            if (absentDays < 0) absentDays = 0;
        }
        
        data.normalHours = normalDays.size();
        data.holidayHours = holidayDays.size();
        data.normalSalary = data.normalHours * base;
        data.holidaySalary = data.holidayHours * base * 2;
        data.penalty = absentDays * PENALTY;

        data.totalSalary = data.normalSalary + data.holidaySalary - data.penalty;
    } else if (role == "Staff" || !isFixedEmployee) {
        for (int hours : normalDays.values()) {
            data.normalHours += hours;
        }
        for (int hours : holidayDays.values()) {
            data.holidayHours += hours;
        }
        data.normalSalary = data.normalHours * base;
        data.holidaySalary = data.holidayHours * base * 2;
        data.penalty = 0; 
        data.totalSalary = data.normalSalary + data.holidaySalary - data.penalty;
    }
    return data;
}

QMap<QString, int> Salary_Model::getNormalDaysData(short int id, QString role, double base, int month, int year)
{
    QMap<QString, int> data;

    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);

    QDate startDate(year, month, 1);
    QDate endDate = QDate(year, month + 1, 1).addDays(-1);
    // if (endDate > QDate::currentDate()) endDate = QDate::currentDate(); // Fix: Allow viewing future shifts in the month

    //qDebug() << "id: " << user->getIdEmployee();

    query.prepare("SELECT * FROM SHIFT WHERE idEmployee = :id "
                  "AND workDate BETWEEN :start AND :end AND status = 1 AND isHoliday = 0");
    query.bindValue(":id", id);
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

QMap<QString, int> Salary_Model::getHolidayDaysData(short int id, QString role, double base, int month, int year)
{
    QMap<QString, int> data;

    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);

    QDate startDate(year, month, 1);
    QDate endDate = QDate(year, month + 1, 1).addDays(-1);
    // if (endDate > QDate::currentDate()) endDate = QDate::currentDate(); // Fix: Allow viewing future shifts in the month

    //qDebug() << "id: " << user->getIdEmployee();

    query.prepare("SELECT * FROM SHIFT WHERE idEmployee = :id "
                  "AND workDate BETWEEN :start AND :end AND status = 1 AND isHoliday = 1");
    query.bindValue(":id", id);
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

