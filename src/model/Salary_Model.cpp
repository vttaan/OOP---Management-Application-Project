#include "Salary_Model.h"

Salary_Model::Salary_Model(QObject *parent) : QObject(parent)
{
}

Salary_Model::~Salary_Model()
{
}

SalaryData Salary_Model::getSalarySummary(int month, int year)
{
    Q_UNUSED(month)
    Q_UNUSED(year)
    // Dummy implementation
    SalaryData data;
    data.normalHours = 120;
    data.holidayHours = 16;
    data.normalSalary = "3.000.000 VNĐ";
    data.holidaySalary = "800.000 VNĐ";
    data.penalty = "50.000 VNĐ";
    data.totalSalary = "3.750.000 VNĐ";
    return data;
}

QMap<QString, int> Salary_Model::getNormalDaysData(short int id, int month, int year)
{
    Q_UNUSED(month)
    Q_UNUSED(year)
    // Dummy implementation
    QMap<QString, int> data;

    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);
    QDate currentDate = QDate::currentDate();
    pair<QDate, QDate> monthRange = {QDate(01, currentDate.month(), currentDate.year()), currentDate};
    query.prepare("SELECT * FROM SHIFT WHERE idEmployee = :id AND workDate BETWEEN :start AND :end");
    query.bindValue(":id", id);
    query.bindValue(":start", monthRange.first);
    query.bindValue(":end", monthRange.second);
    if (!query.exec())
        return data;
    while (query.next())
    {
        QDate date = query.value("workDate").toDate();
        Shift *newShift = new Shift(query.value("idEmployee").toInt(), date,
                                    query.value("startTime").toTime(), query.value("endTime").toTime());
        int dayInWeek = weekRange.first.daysTo(date); // monday: 0, sunday: 6
        if (dayInWeek >= 0 && dayInWeek < 7)
        {
            this->shiftList[dayInWeek].append(newShift);
            this->numberOfShift++;
        }
    }

    data["01/05"] = 8;
    data["02/05"] = 8;
    data["03/05"] = 8;
    data["04/05"] = 4;
    return data;
}

QMap<QString, int> Salary_Model::getHolidayDaysData(int month, int year)
{
    Q_UNUSED(month)
    Q_UNUSED(year)
    // Dummy implementation
    QMap<QString, int> data;
    data["30/04"] = 8;
    data["01/05"] = 8;
    return data;
}

QString Salary_Model::getBaseSalary()
{
    return "25.000 VNĐ"; // Assuming hourly rate or fixed LCB
}
