#include "global.h"
#include "Dashboard_Model.h"

Dashboard_Model::Dashboard_Model() {}

// ---------------------------------------------------------------------------
// Return a set of employee IDs currently scheduled for today's active shift.
// The store operates from 07:00 to 22:00, divided into 3 fixed shift blocks:
// Morning (7-12), Afternoon (12-17), and Evening (17-22).
// This function retrieves ALL employees whose scheduled hours overlap with the current block.
// ---------------------------------------------------------------------------
QSet<int> Dashboard_Model::getWorkingEmployeeIds()
{
    QSet<int> ids;
    QTime now = QTime::currentTime();
    
    QString blockStart, blockEnd;
    if (now >= QTime(7, 0) && now < QTime(12, 0)) {
        blockStart = "07:00:00"; blockEnd = "12:00:00"; // Morning block
    } else if (now >= QTime(12, 0) && now < QTime(17, 0)) {
        blockStart = "12:00:00"; blockEnd = "17:00:00"; // Afternoon block
    } else if (now >= QTime(17, 0) && now < QTime(22, 0)) {
        blockStart = "17:00:00"; blockEnd = "22:00:00"; // Evening block
    } else {
        // Outside of operating hours (00:00-07:00 or 22:00-24:00) -> store is closed
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


// ---------------------------------------------------------------------------
// Return employees scheduled for the next shift block, including their avatar path.
// The store has 3 fixed shifts: 07:00-12:00, 12:00-17:00, 17:00-22:00.
// The next shift is strictly defined chronologically in sequence following the current time.
// ---------------------------------------------------------------------------
QList<ShiftEmployeeInfo> Dashboard_Model::getNextShiftEmployees()
{
    QList<ShiftEmployeeInfo> result;
    QSqlDatabase db = Database::getInstance()->getDbConnect();
    
    QTime now = QTime::currentTime();
    QDate today = QDate::currentDate();
    QString nextDate = today.toString(Qt::ISODate);
    QString nextBlockStart, nextBlockEnd;

    // Calculate the exact start time and date of the next logical shift block: Morning -> Afternoon -> Evening -> Morning
    if (now < QTime(7, 0)) {
        nextBlockStart = "07:00:00"; nextBlockEnd = "12:00:00"; // Currently early morning -> next shift is today's morning shift
    } else if (now < QTime(12, 0)) {
        nextBlockStart = "12:00:00"; nextBlockEnd = "17:00:00"; // Currently in morning shift -> next shift is afternoon
    } else if (now < QTime(17, 0)) {
        nextBlockStart = "17:00:00"; nextBlockEnd = "22:00:00"; // Currently in afternoon shift -> next shift is evening
    } else {
        nextBlockStart = "07:00:00"; nextBlockEnd = "12:00:00"; // After 17:00 (evening shift or closed) -> next shift is tomorrow morning
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


// ---------------------------------------------------------------------------
// Return salary statistics comparing (year-1) vs (year), grouped by month.
// ---------------------------------------------------------------------------
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
