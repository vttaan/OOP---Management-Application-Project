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


void Dashboard_Model::clearCache() {
    m_statsCache.clear();
}

SalaryChartData Dashboard_Model::getSalaryStats(int year)
{
    if (m_statsCache.contains(year)) {
        return m_statsCache[year];
    }

    SalaryChartData data;
    data.lastYearMonthly.fill(0.0, 12);
    data.thisYearMonthly.fill(0.0, 12);
    data.lastYearEmpCount = 0;
    data.thisYearEmpCount = 0;

    for (int y : {year - 1, year}) {
        QString yearStr = QString::number(y);
        QSqlDatabase db = Database::getInstance()->getDbConnect();
        
        // 1. Get total part-time salary per month using SQL Aggregation
        QSqlQuery ptQuery(db);
        ptQuery.prepare(
            "SELECT "
            "    CAST(strftime('%m', S.workDate) AS INTEGER) AS month, "
            "    SUM((julianday(S.endTime) - julianday(S.startTime)) * 24.0 * P.Salary * (CASE WHEN S.isHoliday THEN 2 ELSE 1 END)) AS ptSalary "
            "FROM SHIFT S JOIN PROFILES P ON S.idEmployee = P.idEmployee "
            "WHERE S.status = 1 AND strftime('%Y', S.workDate) = :yr AND P.isFixed = 0 "
            "GROUP BY month"
        );
        ptQuery.bindValue(":yr", yearStr);
        if (ptQuery.exec()) {
            while (ptQuery.next()) {
                int month = ptQuery.value(0).toInt() - 1; // 0-11
                if (month >= 0 && month < 12) {
                    if (y == year) data.thisYearMonthly[month] += ptQuery.value(1).toDouble();
                    else           data.lastYearMonthly[month] += ptQuery.value(1).toDouble();
                }
            }
        }

        // 2. Get total full-time salary per month
        QSqlQuery ftQuery(db);
        ftQuery.prepare(
            "SELECT month, SUM(Salary) AS ftSalary "
            "FROM ( "
            "    SELECT DISTINCT S.idEmployee, P.Salary, CAST(strftime('%m', S.workDate) AS INTEGER) AS month "
            "    FROM SHIFT S JOIN PROFILES P ON S.idEmployee = P.idEmployee "
            "    WHERE S.status = 1 AND strftime('%Y', S.workDate) = :yr AND P.isFixed = 1 "
            ") "
            "GROUP BY month"
        );
        ftQuery.bindValue(":yr", yearStr);
        if (ftQuery.exec()) {
            while (ftQuery.next()) {
                int month = ftQuery.value(0).toInt() - 1; // 0-11
                if (month >= 0 && month < 12) {
                    if (y == year) data.thisYearMonthly[month] += ftQuery.value(1).toDouble();
                    else           data.lastYearMonthly[month] += ftQuery.value(1).toDouble();
                }
            }
        }

        // 3. Get total distinct employees for the year
        QSqlQuery empQuery(db);
        empQuery.prepare(
            "SELECT COUNT(DISTINCT idEmployee) "
            "FROM SHIFT "
            "WHERE status = 1 AND strftime('%Y', workDate) = :yr"
        );
        empQuery.bindValue(":yr", yearStr);
        if (empQuery.exec() && empQuery.next()) {
            if (y == year) data.thisYearEmpCount = empQuery.value(0).toInt();
            else           data.lastYearEmpCount = empQuery.value(0).toInt();
        }
    }

    m_statsCache[year] = data;
    return data;
}
