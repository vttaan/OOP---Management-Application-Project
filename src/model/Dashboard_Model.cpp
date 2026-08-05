#include "global.h"
#include "Dashboard_Model.h"
#include "utils/Config.h"

Dashboard_Model::Dashboard_Model() {}

// Gets the employee IDs working in the current shift.
// Shift 1: openHour to 12:00
// Shift 2: 12:00 to 17:00
// Shift 3: 17:00 to closeHour
// Called by: Dashboard_Control::loadEmployeeCards()
// Calls: Config::getOpenHour(), Config::getCloseHour(), QTime::currentTime(), QDate::currentDate(), Database::getInstance(), QSqlQuery::exec()
QSet<int> Dashboard_Model::getWorkingEmployeeIds()
{
    int openHour = Config::getOpenHour();
    int closeHour = Config::getCloseHour();

    QSet<int> ids;
    QTime now = QTime::currentTime();
    
    QString blockStart, blockEnd;
    if (now >= QTime(openHour, 0) && now < QTime(12, 0)) {
        blockStart = QString("%1:00:00").arg(openHour, 2, 10, QChar('0'));
        blockEnd = "12:00:00";
    } else if (now >= QTime(12, 0) && now < QTime(17, 0)) {
        blockStart = "12:00:00";
        blockEnd = "17:00:00";
    } else if (now >= QTime(17, 0) && now < QTime(closeHour, 0)) {
        blockStart = "17:00:00";
        blockEnd = QString("%1:00:00").arg(closeHour, 2, 10, QChar('0'));
    } else {
        // Outside of operating hours
        return ids; 
    }

    QSqlQuery q(Database::getInstance()->getDbConnect());
    q.prepare(
        "SELECT DISTINCT idEmployee FROM SHIFT "
        "WHERE status = 1 "
        "  AND workDate = :today "
        "  AND startTime < :blockEnd "
        "  AND endTime > :blockStart"
    );
    q.bindValue(":today", QDate::currentDate().toString(Qt::ISODate));
    q.bindValue(":blockStart", blockStart);
    q.bindValue(":blockEnd", blockEnd);

    if (q.exec())
        while (q.next())
            ids.insert(q.value(0).toInt());
    return ids;
}


// Gets the employee information for the next logical shift.
// Determines the next shift block based on the current time and returns employees scheduled for it.
// Called by: Dashboard_Control::loadShiftPanel()
// Calls: Config::getOpenHour(), Config::getCloseHour(), QTime::currentTime(), QDate::currentDate(), Database::getInstance(), QSqlQuery::exec()
QList<ShiftEmployeeInfo> Dashboard_Model::getNextShiftEmployees()
{
    QList<ShiftEmployeeInfo> result;
    QSqlDatabase db = Database::getInstance()->getDbConnect();
    
    int openHour = Config::getOpenHour();
    int closeHour = Config::getCloseHour();

    QTime now = QTime::currentTime();
    QDate today = QDate::currentDate();
    QString nextDate = today.toString(Qt::ISODate);
    QString nextBlockStart, nextBlockEnd;

    // Calculate the exact start time and date of the next logical shift block
    if (now < QTime(openHour, 0)) {
        nextBlockStart = QString("%1:00:00").arg(openHour, 2, 10, QChar('0'));
        nextBlockEnd = "12:00:00";
    } else if (now < QTime(12, 0)) {
        nextBlockStart = "12:00:00";
        nextBlockEnd = "17:00:00";
    } else if (now < QTime(17, 0)) {
        nextBlockStart = "17:00:00";
        nextBlockEnd = QString("%1:00:00").arg(closeHour, 2, 10, QChar('0'));
    } else {
        nextBlockStart = QString("%1:00:00").arg(openHour, 2, 10, QChar('0'));
        nextBlockEnd = "12:00:00";
        nextDate = today.addDays(1).toString(Qt::ISODate);
    }

    // Fetch all employees whose shifts strictly overlap with the determined next chronological block
    QSqlQuery q(db);
    q.prepare(
        "SELECT DISTINCT P.name, P.phoneNum, P.role, P.avatarPath "
        "FROM SHIFT S JOIN PROFILES P ON S.idEmployee = P.idEmployee "
        "WHERE S.workDate = :ndate "
        "  AND S.startTime < :nextBlockEnd "
        "  AND S.endTime > :nextBlockStart "
        "  AND S.status = 1 "
        "ORDER BY P.name"
    );
    q.bindValue(":ndate", nextDate);
    q.bindValue(":nextBlockStart", nextBlockStart);
    q.bindValue(":nextBlockEnd", nextBlockEnd);

    if (q.exec()) {
        while (q.next()) {
            ShiftEmployeeInfo info;
            info.name       = q.value(0).toString();
            info.phone      = q.value(1).toString();
            info.role       = q.value(2).toString();
            info.avatarPath     = q.value(3).toString();
            result.append(info);
        }
    }
    return result;
}


// Calculates salary statistics for the given year and the previous year.
// Aggregates total salary per month, accounting for fixed salaries and hourly wages with holiday multipliers.
// Called by: Dashboard_Control::loadSalaryChart()
// Calls: Database::getInstance(), QSqlQuery::exec(), QDate::daysInMonth()
SalaryChartData Dashboard_Model::getSalaryStats(int year)
{
    SalaryChartData data;
    data.lastYearMonthly.fill(0.0, 12);
    data.thisYearMonthly.fill(0.0, 12);
    data.lastYearEmpCount = 0;
    data.thisYearEmpCount = 0;

    for (int y : {year - 1, year}) {
        QSqlQuery q(Database::getInstance()->getDbConnect());
        q.prepare(
            "SELECT S.idEmployee, P.role, P.Salary, P.isFixed, "
            "       CAST(strftime('%m', S.workDate) AS INTEGER), "
            "       S.startTime, S.endTime, S.isHoliday "
            "FROM SHIFT S JOIN PROFILES P ON S.idEmployee = P.idEmployee "
            "WHERE S.status = 1 AND strftime('%Y', S.workDate) = :yr"
        );
        if (!q.lastError().text().isEmpty())
            qDebug() << "[Dashboard] prepare() error year=" << y << q.lastError().text();
        q.bindValue(":yr", QString::number(y));
        
        QSet<int> yearEmployees;
        
        struct EmpData {
            QString role;
            double base;
            bool isFixed;
            double hours = 0;
        };
        
        // Map from month (0-11) -> map from idEmployee -> EmpData
        QMap<int, QMap<int, EmpData>> monthData;
        
        if (q.exec()) {
            int rowCount = 0;
            while (q.next()) {
                rowCount++;
                int id       = q.value(0).toInt();
                QString role = q.value(1).toString();
                double base  = q.value(2).toDouble();
                bool isFixed = q.value(3).toBool();
                int m        = q.value(4).toInt() - 1;
                if (m < 0 || m > 11) continue;
                
                double hrs        = q.value(5).toTime().secsTo(q.value(6).toTime()) / 3600.0;
                double multiplier = q.value(7).toBool() ? 2.0 : 1.0;

                yearEmployees.insert(id);
                
                if (!monthData[m].contains(id)) {
                    monthData[m][id] = {role, base, isFixed, 0.0};
                }
                monthData[m][id].hours += (hrs * multiplier);
            }
            qDebug() << "[Dashboard] getSalaryStats year=" << y << "rows=" << rowCount << "employees=" << yearEmployees.size();
        } else {
            qDebug() << "[Dashboard] getSalaryStats QUERY FAILED year=" << y << q.lastError().text();
        }
        
        if (y == year) data.thisYearEmpCount = yearEmployees.size();
        else           data.lastYearEmpCount = yearEmployees.size();
        
        for (int m = 0; m < 12; ++m) {
            double totalMonthSalary = 0;
            int daysInMonth = QDate(y, m + 1, 1).daysInMonth();
            
            for (auto it = monthData[m].begin(); it != monthData[m].end(); ++it) {
                const EmpData& emp = it.value();
                if (emp.isFixed) {
                    totalMonthSalary += emp.base;
                } else {
                    totalMonthSalary += emp.hours * emp.base;
                }
            }
            
            if (y == year) data.thisYearMonthly[m] = totalMonthSalary;
            else           data.lastYearMonthly[m] = totalMonthSalary;
        }
    }
    return data;
}
