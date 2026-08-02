#include "global.h"
#include "Dashboard_Model.h"

Dashboard_Model::Dashboard_Model() {}

// ---------------------------------------------------------------------------
// Determine shift boundaries from the current system time.
// Shifts: Morning 07:00-12:00 | Afternoon 12:00-17:00 | Evening 17:00-22:00
// ---------------------------------------------------------------------------
void Dashboard_Model::resolveShifts(QString& currentStart,
                                    QString& nextStart,
                                    QString& nextDate) const
{
    QTime now = QTime::currentTime();
    nextDate  = QDate::currentDate().toString(Qt::ISODate);

    if (now >= QTime(7, 0) && now < QTime(12, 0)) {
        currentStart = "07:00:00";
        nextStart    = "12:00:00";
    } else if (now >= QTime(12, 0) && now < QTime(17, 0)) {
        currentStart = "12:00:00";
        nextStart    = "17:00:00";
    } else {
        currentStart = "17:00:00";
        nextStart    = "07:00:00";
        if (now >= QTime(17, 0))
            nextDate = QDate::currentDate().addDays(1).toString(Qt::ISODate);
    }
}

// ---------------------------------------------------------------------------
// Insert 4 employees from PROFILES into today's current shift and next shift,
// only if those records do not already exist.
// ---------------------------------------------------------------------------
void Dashboard_Model::seedTodayShifts()
{
    QSqlDatabase db = Database::getInstance()->getDbConnect();
    QSqlQuery q(db);

    // Fetch up to 8 employee IDs to split between current and next shift
    QVector<int> ids;
    q.exec("SELECT idEmployee FROM PROFILES LIMIT 8");
    while (q.next()) ids.append(q.value(0).toInt());
    if (ids.size() < 1) return;

    QString currentStart, nextStart, nextDate;
    resolveShifts(currentStart, nextStart, nextDate);

    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    auto endTime = [](const QString& start) -> QString {
        if (start == "07:00:00") return "12:00:00";
        if (start == "12:00:00") return "17:00:00";
        return "22:00:00";
    };

    // Insert current shift records for first 4 employees
    int half = qMin(4, (int)ids.size());
    for (int i = 0; i < half; ++i) {
        int id = ids[i];
        q.prepare(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status, isHoliday) "
            "SELECT :id, :date, :start, :end, 1, 0 "
            "WHERE NOT EXISTS ("
            "  SELECT 1 FROM SHIFT "
            "  WHERE idEmployee = :id2 AND workDate = :date2 AND startTime = :start2"
            ")"
        );
        q.bindValue(":id",     id);   q.bindValue(":date",   today);
        q.bindValue(":start",  currentStart);
        q.bindValue(":end",    endTime(currentStart));
        q.bindValue(":id2",    id);   q.bindValue(":date2",  today);
        q.bindValue(":start2", currentStart);
        q.exec();
    }

    // Insert next shift records for the remaining employees (up to 4)
    for (int i = half; i < ids.size(); ++i) {
        int id = ids[i];
        q.prepare(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status, isHoliday) "
            "SELECT :id, :date, :start, :end, 1, 0 "
            "WHERE NOT EXISTS ("
            "  SELECT 1 FROM SHIFT "
            "  WHERE idEmployee = :id2 AND workDate = :date2 AND startTime = :start2"
            ")"
        );
        q.bindValue(":id",     id);   q.bindValue(":date",   nextDate);
        q.bindValue(":start",  nextStart);
        q.bindValue(":end",    endTime(nextStart));
        q.bindValue(":id2",    id);   q.bindValue(":date2",  nextDate);
        q.bindValue(":start2", nextStart);
        q.exec();
    }
}

// ---------------------------------------------------------------------------
// Return a set of employee IDs currently scheduled for today's active shift.
// ---------------------------------------------------------------------------
QSet<int> Dashboard_Model::getWorkingEmployeeIds()
{
    QSet<int> ids;
    QSqlQuery q(Database::getInstance()->getDbConnect());
    
    // Check if employee has a shift that is currently ongoing
    // Handles normal shifts (startTime < endTime) and midnight-crossing shifts (startTime >= endTime)
    q.prepare(
        "SELECT DISTINCT idEmployee FROM SHIFT "
        "WHERE status = 1 AND ("
        "  (workDate = :today AND startTime < endTime AND startTime <= :now AND endTime > :now) OR "
        "  (workDate = :today AND startTime >= endTime AND startTime <= :now) OR "
        "  (workDate = :yesterday AND startTime >= endTime AND endTime > :now)"
        ")"
    );
    q.bindValue(":today", QDate::currentDate().toString(Qt::ISODate));
    q.bindValue(":yesterday", QDate::currentDate().addDays(-1).toString(Qt::ISODate));
    q.bindValue(":now", QTime::currentTime().toString("HH:mm:ss"));
    
    if (q.exec())
        while (q.next())
            ids.insert(q.value(0).toInt());
    return ids;
}

// ---------------------------------------------------------------------------
// Return employees scheduled for the next shift, including their avatar path.
// ---------------------------------------------------------------------------
QList<ShiftEmployeeInfo> Dashboard_Model::getNextShiftEmployees()
{
    QList<ShiftEmployeeInfo> result;
    QSqlDatabase db = Database::getInstance()->getDbConnect();
    
    // 1. Find the start time and date of the next upcoming shift block
    QSqlQuery findNext(db);
    findNext.prepare(
        "SELECT workDate, startTime FROM SHIFT "
        "WHERE status = 1 AND ("
        "  (workDate = :today AND startTime > :now) OR "
        "  (workDate > :today)"
        ") "
        "ORDER BY workDate ASC, startTime ASC "
        "LIMIT 1"
    );
    findNext.bindValue(":today", QDate::currentDate().toString(Qt::ISODate));
    findNext.bindValue(":now", QTime::currentTime().toString("HH:mm:ss"));
    
    QString nextDate, nextStart;
    if (findNext.exec() && findNext.next()) {
        nextDate = findNext.value(0).toString();
        nextStart = findNext.value(1).toString();
    } else {
        return result; // No upcoming shifts scheduled
    }

    // 2. Fetch all employees belonging to that specific next shift
    QSqlQuery q(db);
    q.prepare(
        "SELECT P.name, P.phoneNum, P.role, P.avatarPath "
        "FROM SHIFT S JOIN PROFILES P ON S.idEmployee = P.idEmployee "
        "WHERE S.workDate = :ndate AND S.startTime = :nst AND S.status = 1 "
        "ORDER BY P.name"
    );
    q.bindValue(":ndate", nextDate);
    q.bindValue(":nst",   nextStart);
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
// Return names of employees absent from today's current shift.
// Combines approved leave requests and no check-in timekeeping records.
// ---------------------------------------------------------------------------
QStringList Dashboard_Model::getAbsentEmployees()
{
    QStringList names;
    QSqlQuery q(Database::getInstance()->getDbConnect());
    q.prepare(
        "SELECT P.name, 'Nghi phep' AS reason "
        "FROM PROFILES P "
        "JOIN LEAVE_REQUEST L ON P.idEmployee = L.idEmployee "
        "WHERE L.leaveDate = :today AND L.status = 'Approved' "
        "UNION "
        "SELECT P.name, 'Vang mat' AS reason "
        "FROM PROFILES P "
        "JOIN SHIFT S ON P.idEmployee = S.idEmployee "
        "WHERE S.status = 1 AND ("
        "  (S.workDate = :today AND S.startTime < S.endTime AND S.startTime <= :now AND S.endTime > :now) OR "
        "  (S.workDate = :today AND S.startTime >= S.endTime AND S.startTime <= :now) OR "
        "  (S.workDate = :yesterday AND S.startTime >= S.endTime AND S.endTime > :now)"
        ") "
        "AND P.idEmployee NOT IN ("
        "  SELECT idEmployee FROM TIMEKEEPING WHERE checkInDate = :today OR checkInDate = :yesterday"
        ")"
    );
    q.bindValue(":today", QDate::currentDate().toString(Qt::ISODate));
    q.bindValue(":yesterday", QDate::currentDate().addDays(-1).toString(Qt::ISODate));
    q.bindValue(":now", QTime::currentTime().toString("HH:mm:ss"));
    
    if (q.exec()) {
        while (q.next()) {
            names << q.value(0).toString() + " (" + q.value(1).toString() + ")";
        }
    }
    return names;
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
            while (q.next()) {
                int id = q.value(0).toInt();
                QString role = q.value(1).toString();
                double base = q.value(2).toDouble();
                bool isFixed = q.value(3).toBool();
                int m = q.value(4).toInt() - 1;
                if (m < 0 || m > 11) continue;
                
                double hrs = q.value(5).toTime().secsTo(q.value(6).toTime()) / 3600.0;
                double multiplier = q.value(7).toBool() ? 2.0 : 1.0; // Holiday pays double

                yearEmployees.insert(id);
                
                if (!monthData[m].contains(id)) {
                    monthData[m][id] = {role, base, isFixed, 0.0};
                }
                monthData[m][id].hours += (hrs * multiplier);
            }
        }
        
        if (y == year) data.thisYearEmpCount = yearEmployees.size();
        else           data.lastYearEmpCount = yearEmployees.size();
        
        for (int m = 0; m < 12; ++m) {
            double totalMonthSalary = 0;
            int daysInMonth = QDate(y, m + 1, 1).daysInMonth();
            
            for (auto it = monthData[m].begin(); it != monthData[m].end(); ++it) {
                const EmpData& emp = it.value();
                if (emp.isFixed) {
                    if (emp.role == "Manager" || emp.role == "Admin") {
                        totalMonthSalary += emp.base * daysInMonth;
                    } else {
                        totalMonthSalary += emp.base;
                    }
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
