#include "Schedule_Model.h"

Schedule_Model::Schedule_Model() : numberOfShift(0) {}

bool Schedule_Model::checkOverlapping(short int id, QDate date, QTime start, QTime end)
{
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);
    query.prepare("SELECT * FROM SHIFT WHERE idEmployee = :id AND workDate = :date");
    query.bindValue(":id", id);
    query.bindValue(":date", date);

    if (!query.exec())
        return true;
    while (query.next())
    {
        QTime currentStartTime = query.value("startTime").toTime();
        QTime currentEndTime = query.value("endTime").toTime();
        if (start < currentEndTime && currentStartTime < end)
            return false;
    }
    return true;
}

Shift *Schedule_Model::getPreviewShift(short int id, QDate date, QTime start, QTime end)
{
    return new Shift(id, date, start, end);
}

pair<QDate, QDate> Schedule_Model::getRangeOfWeek()
{
    QDate monday = QDate::currentDate().addDays(-QDate::currentDate().dayOfWeek() + 1);
    QDate sunday = monday.addDays(6);
    return {monday, sunday};
}

void Schedule_Model::getSchedule(short int id)
{
    for (int i = 0; i < 7; i++)
    {
        qDeleteAll(this->shiftList[i]);
        this->shiftList[i].clear();
    }
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);
    pair<QDate, QDate> weekRange = getRangeOfWeek();
    query.prepare("SELECT * FROM SHIFT WHERE idEmployee = :id AND workDate BETWEEN :start AND :end");
    query.bindValue(":id", id);
    query.bindValue(":start", weekRange.first);
    query.bindValue(":end", weekRange.second);
    if (!query.exec())
        return;
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
}

// when a user create a shift, a temporary shift is created and add to the system's sort algorithm,
// only then the database will add approved shifts

// flow in control: select shift -> model check overlapping -> return preview -> submit -> system handle -> add shift to database


// temporary solution, might fix later o_O
QList<QString> holidayList = {"01/01", "30/04", "01/05", "02/09"};

Shift *Schedule_Model::handleAddShiftSubmission(short int id, QDate date, QTime start, QTime end)
{
    // checkOverlapping returns true when the slot is FREE (no overlap), false when blocked
    if (!checkOverlapping(id, date, start, end))
    {
        qDebug() << "Overlapped — shift rejected";
        return nullptr;
    }

    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    if (!openData.transaction())
    {
        qDebug() << "Failed to start transaction";
        return nullptr;
    }
    QSqlQuery query(openData);
    query.prepare(
        "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status, isHoliday) "
        "VALUES (:id,:date,:start,:end,:status,:isHoliday)");
    query.bindValue(":id", id);
    query.bindValue(":date", date);
    query.bindValue(":start", start);
    query.bindValue(":end", end);
    query.bindValue(":status", 0);
    query.bindValue(":isHoliday", 0);
    if (holidayList.contains(QString::number(date.day()) + "/" + QString::number(date.month()))) {
        query.bindValue("isHoliday", 1);
    }
    if (!query.exec())
    {
        qDebug() << "Error adding shift" << query.lastError().text();
        openData.rollback();
        return nullptr;
    }
    openData.commit();
    Shift *newShift = new Shift(id, date, start, end);
    return newShift;
}

Schedule_Model::~Schedule_Model()
{
    for (auto &dayShifts : shiftList)
    {
        qDeleteAll(dayShifts);
    }
}
QVector<ShiftRegistration>
Schedule_Model::fetchPendingShifts(const QDate& weekStart, const QDate& weekEnd) {
    QVector<ShiftRegistration> regs;
    QSqlQuery q(Database::getInstance()->getDbConnect());
    q.prepare("SELECT rowid, idEmployee, workDate, startTime, endTime "
              "FROM SHIFT "
              "WHERE status = 0 "
              "  AND workDate >= :start "
              "  AND workDate <= :end");
    q.bindValue(":start", weekStart.toString(Qt::ISODate));
    q.bindValue(":end",   weekEnd.toString(Qt::ISODate));
    if (!q.exec()) return regs;
    while (q.next()) {
        ShiftRegistration r;
        r.rowId      = q.value(0).toInt();
        r.employeeId = q.value(1).toInt();
        r.date       = QDate::fromString(q.value(2).toString(), Qt::ISODate);
        r.startTime  = QTime::fromString(q.value(3).toString(), "H:mm");
        r.endTime    = QTime::fromString(q.value(4).toString(), "H:mm");
        regs.push_back(r);
    }
    return regs;
}
QVector<EmployeeInfo>
Schedule_Model::fetchAllEmployeeInfos(const QDate& weekStart) {
    QVector<int> allIds;
    {
        QSqlQuery q(Database::getInstance()->getDbConnect());
        q.prepare("SELECT DISTINCT idEmployee FROM PROFILES");
        if (q.exec())
            while (q.next()) allIds.push_back(q.value(0).toInt());
    }
    // 2. Tính tổng phút đã làm (status=1) trước tuần này
    QMap<int, int> minutesMap;
    {
        QSqlQuery q(Database::getInstance()->getDbConnect());
        q.prepare("SELECT idEmployee, startTime, endTime "
                  "FROM SHIFT "
                  "WHERE status = 1 AND workDate < :weekStart");
        q.bindValue(":weekStart", weekStart.toString(Qt::ISODate));
        if (q.exec()) {
            while (q.next()) {
                int   id    = q.value(0).toInt();
                QTime start = QTime::fromString(q.value(1).toString(), "H:mm");
                QTime end   = QTime::fromString(q.value(2).toString(), "H:mm");
                if (start.isValid() && end.isValid())
                    minutesMap[id] += start.secsTo(end) / 60;
            }
        }
    }
    QVector<EmployeeInfo> infos;
    infos.reserve(allIds.size());
    for (int id : allIds)
        infos.push_back({id, minutesMap.value(id, 0)});
    return infos;
}
OptimizerOutput Schedule_Model::generateSchedule() {
    QDate today     = QDate::currentDate();
    QDate weekStart = today.addDays(1 - today.dayOfWeek());
    QDate weekEnd   = weekStart.addDays(6);
    OptimizerInput input;
    input.registrations = fetchPendingShifts(weekStart, weekEnd);
    input.employees     = fetchAllEmployeeInfos(weekStart);
    input.minPerShift   = 5;
    input.minDaysPerEmp = 4;
    Optimizer opt;
    OptimizerOutput output = opt.solve(input);
    if (output.feasible && !output.assignments.isEmpty()) {
        QSqlDatabase db = Database::getInstance()->getDbConnect();
        db.transaction();
        QSqlQuery q(db);
        q.prepare("UPDATE SHIFT SET status = :status WHERE rowid = :rowid");
        for (const auto& a : output.assignments) {
            q.bindValue(":status", a.newStatus);
            q.bindValue(":rowid",  a.rowId);
            if (!q.exec())
                output.warnings << QString("[DB Error] rowid=%1: %2")
                                       .arg(a.rowId).arg(q.lastError().text());
        }
        db.commit();
    }
    return output;
}