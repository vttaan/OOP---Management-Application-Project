#include "Schedule_Model.h"
#include "core/UserFactory.h"

Schedule_Model::Schedule_Model() : numberOfShift(0) {}

bool Schedule_Model::checkOverlapping(short int id, QDate date, QTime start, QTime end)
{
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);
    query.prepare("SELECT * FROM SHIFT WHERE idEmployee = :id AND workDate = :date");
    query.bindValue(":id", id);
    query.bindValue(":date", date);

    if (query.exec()) {
        while (query.next())
        {
            QTime currentStartTime = query.value("startTime").toTime();
            QTime currentEndTime = query.value("endTime").toTime();
            if (start < currentEndTime && currentStartTime < end)
                return false;
        }
    }


    for (Shift* draft : draftShifts) {
        if (draft->getEmployeeID() == id && draft->getDate() == date) {
            if (start < draft->getEndTime() && draft->getStartTime() < end) {
                return false;
            }
        }
    }
    return true;
}

Shift *Schedule_Model::getPreviewShift(short int id, QDate date, QTime start, QTime end)
{
    return new Shift(id, date, start, end);
}

void Schedule_Model::getSchedule(short int id, QDate monday)
{
    for (int i = 0; i < 7; i++)
    {
        qDeleteAll(this->shiftList[i]);
        this->shiftList[i].clear();
    }
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);
    QDate sunday = monday.addDays(6);
    query.prepare("SELECT * FROM SHIFT WHERE idEmployee = :id AND workDate BETWEEN :start AND :end");
    query.bindValue(":id", id);
    query.bindValue(":start", monday);
    query.bindValue(":end", sunday);
    if (!query.exec())
        return;
    while (query.next())
    {
        QDate date = query.value("workDate").toDate();
        Shift *newShift = new Shift(query.value("idEmployee").toInt(), date,
                                    query.value("startTime").toTime(), query.value("endTime").toTime());
        int dayInWeek = monday.daysTo(date); // monday: 0, sunday: 6
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

    Shift *newShift = new Shift(id,date,start,end);
    newShift->setShiftId(-1); // note for draftshift
    draftShifts.append(newShift);

    QDate monday = date.addDays(1 - date.dayOfWeek());
    int dayInWeek = monday.daysTo(date);
    if (dayInWeek >= 0 && dayInWeek < 7) {
        this->shiftList[dayInWeek].append(newShift);
        this->numberOfShift++;
    }

    return newShift;
}

void Schedule_Model::getAcceptedSchedule(short int id, QDate monday)
{
    for (int i = 0; i < 7; i++)
    {
        qDeleteAll(this->shiftList[i]);
        this->shiftList[i].clear();
    }

    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);
    QDate sunday = monday.addDays(6);

    query.prepare("SELECT * FROM SHIFT WHERE idEmployee = :id AND status = 1 AND workDate BETWEEN :start AND :end");
    query.bindValue(":id", id);
    query.bindValue(":start", monday);
    query.bindValue(":end", sunday);

    if (!query.exec())
        return;

    while (query.next())
    {
        QDate date = query.value("workDate").toDate();
        Shift *newShift = new Shift(id, date,
                                    query.value("startTime").toTime(),
                                    query.value("endTime").toTime());

        int dayInWeek = monday.daysTo(date); // monday: 0, sunday: 6
        if (dayInWeek >= 0 && dayInWeek < 7)
        {
            this->shiftList[dayInWeek].append(newShift);
            this->numberOfShift++;
        }
    }
}

QMap<int, QMap<int, ShiftBlock *>> Schedule_Model::getManagerWeeklyGrid(QDate monday, int status)
{
    qDeleteAll(currentWeeklyUsers);
    currentWeeklyUsers.clear();

    QMap<int, QMap<int, ShiftBlock *>> grid;
    QDate sunday = monday.addDays(6);

    // Initialize 7x14 grid
    for (int col = 0; col < 7; ++col)
    {
        QDate currentDate = monday.addDays(col);
        for (int row = 0; row < 14; ++row)
        { // 8:00 to 21:00
            QTime start(8 + row, 0);
            QTime end(8 + row + 1, 0);
            grid[col][row] = new ShiftBlock(currentDate, start, end);
        }
    }

    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);

    query.prepare("SELECT S.*, P.* FROM SHIFT S "
                  "JOIN PROFILES P ON S.idEmployee = P.idEmployee "
                  "WHERE S.status = :status AND S.workDate BETWEEN :start AND :end");
    query.bindValue(":status", status);
    query.bindValue(":start", monday);
    query.bindValue(":end", sunday);

    if (!query.exec())
    {
        qDebug() << "Failed to fetch all accepted schedule:" << query.lastError().text();
        return grid;
    }

    while (query.next())
    {
        User *user = UserFactory::createContainsUser(
            query.value("role").toString(),
            query.value("idEmployee").toInt(),
            query.value("avatarPath").toString(),
            query.value("IdCitizenIdentity").toString(),
            query.value("name").toString(),
            query.value("dob").toString(),
            query.value("address").toString(),
            query.value("phoneNum").toString(),
            query.value("Gender").toString(),
            query.value("Salary").toInt()
        );
        currentWeeklyUsers.append(user);

        QDate workDate = query.value("workDate").toDate();
        QTime startTime = query.value("startTime").toTime();
        QTime endTime = query.value("endTime").toTime();

        int col = monday.daysTo(workDate);
        if (col >= 0 && col < 7)
        {
            for (int row = 0; row < 14; ++row)
            {
                QTime slotStart(8 + row, 0);
                QTime slotEnd(8 + row + 1, 0);
                if (startTime < slotEnd && endTime > slotStart)
                {
                    grid[col][row]->addStaff(user);
                }
            }
        }
    }
    return grid;
}

Schedule_Model::~Schedule_Model()
{
    qDeleteAll(currentWeeklyUsers);
    currentWeeklyUsers.clear();
    for (auto &dayShifts : shiftList)
    {
        qDeleteAll(dayShifts);
    }
}

QMap<int, QList<QString>> Schedule_Model::getWeeklySummaryStrings() const
{
    QMap<int, QList<QString>> weeklyData;
    for (int col = 0; col < shiftList.size(); ++col)
    {
        QList<QString> labels;
        for (const Shift *s : shiftList[col])
        {
            QString label = s->getStartTime().toString("HH:mm") + " - " + s->getEndTime().toString("HH:mm");
            labels.append(label);
        }
        if (!labels.isEmpty())
            weeklyData.insert(col, labels);
    }
    return weeklyData;
}

QVector<Shift *>
Schedule_Model::fetchPendingShifts(const QDate &weekStart, const QDate &weekEnd)
{
    QVector<Shift *> regs;
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
        int shiftId  = q.value(0).toInt();
        int empId    = q.value(1).toInt();
        QDate date   = QDate::fromString(q.value(2).toString(), Qt::ISODate);
        QTime startT = QTime::fromString(q.value(3).toString(), "H:mm");
        QTime endT   = QTime::fromString(q.value(4).toString(), "H:mm");
        Shift *s = new Shift(empId, date, startT, endT);
        s->setShiftId(shiftId);
        regs.push_back(s);
    }
    return regs;
}

QMap<User *, int>
Schedule_Model::fetchAllEmployeeInfos(const QDate &weekStart)
{
    QMap<User *, int> employeeMinutesWorked;

    QMap<int, int> minutesMap;
    {
        QSqlQuery q(Database::getInstance()->getDbConnect());
        q.prepare("SELECT idEmployee, startTime, endTime "
                  "FROM SHIFT "
                  "WHERE status = 1 AND workDate < :weekStart");
        q.bindValue(":weekStart", weekStart.toString(Qt::ISODate));
        if (q.exec())
        {
            while (q.next())
            {
                int id = q.value(0).toInt();
                QTime start = QTime::fromString(q.value(1).toString(), "H:mm");
                QTime end = QTime::fromString(q.value(2).toString(), "H:mm");
                if (start.isValid() && end.isValid())
                    minutesMap[id] += start.secsTo(end) / 60;
            }
        }
    }

    {
        qDeleteAll(currentWeeklyUsers);
        currentWeeklyUsers.clear();
        QSqlQuery q(Database::getInstance()->getDbConnect());
        q.prepare("SELECT * FROM PROFILES");
        if (q.exec()) {
            while (q.next()) {
                User *user = UserFactory::createContainsUser(
                    q.value("role").toString(),
                    q.value("idEmployee").toInt(),
                    q.value("avatarPath").toString(),
                    q.value("IdCitizenIdentity").toString(),
                    q.value("name").toString(),
                    q.value("dob").toString(),
                    q.value("address").toString(),
                    q.value("phoneNum").toString(),
                    q.value("Gender").toString(),
                    q.value("Salary").toInt()
                );
                currentWeeklyUsers.append(user);
                employeeMinutesWorked[user] = minutesMap.value(user->getIdEmployee(), 0);
            }
        }
    }
    return employeeMinutesWorked;
}

QStringList Schedule_Model::generateSchedule()
{
    QDate today     = QDate::currentDate();
    // Thuật toán sẽ luôn chạy cho tuần SAU (Next Week)
    QDate weekStart = today.addDays(8 - today.dayOfWeek());
    QDate weekEnd   = weekStart.addDays(6);

    QVector<Shift *> pendingShifts = fetchPendingShifts(weekStart, weekEnd);
    QMap<User *, int> employeeMinutes = fetchAllEmployeeInfos(weekStart);

    Optimizer opt(pendingShifts, employeeMinutes);
    opt.solve();

    QStringList warnings = opt.getWarnings();

    if (opt.isFeasible() && !pendingShifts.isEmpty()) {
        QSqlDatabase db = Database::getInstance()->getDbConnect();
        db.transaction();
        QSqlQuery q(db);
        q.prepare("UPDATE SHIFT SET status = :status WHERE rowid = :rowid");
        for (Shift *s : pendingShifts) {
            // Chỉ cập nhật nếu status đã được thay đổi (thuật toán xét duyệt)
            if (s->getStatus() != 0) {
                q.bindValue(":status", s->getStatus());
                q.bindValue(":rowid",  s->getShiftId());
                if (!q.exec())
                    warnings << QString("[DB Error] rowid=%1: %2")
                                           .arg(s->getShiftId()).arg(q.lastError().text());
            }
        }
        db.commit();
    }

    qDeleteAll(pendingShifts);
    pendingShifts.clear();

    return warnings;
}

bool Schedule_Model::saveDraftShiftsToDatabase()
{
    if (draftShifts.isEmpty()) return true;
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    if (!openData.transaction()) return false;
    QSqlQuery query(openData);
    query.prepare(
        "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status, isHoliday) "
        "VALUES (:id, :date, :start, :end, :status, :isHoliday)"
        );
    QList<QString> holidayList = {"01/01", "30/04", "01/05", "02/09"};
    for (Shift* shift : draftShifts) {
        query.bindValue(":id", shift->getEmployeeID());
        query.bindValue(":date", shift->getDate());
        query.bindValue(":start", shift->getStartTime());
        query.bindValue(":end", shift->getEndTime());
        query.bindValue(":status", 0); // 0 = Chờ duyệt

        bool isHoliday = holidayList.contains(shift->getDate().toString("dd/MM"));
        query.bindValue(":isHoliday", isHoliday);

        if (!query.exec()) {
            qDebug() << "Lỗi khi lưu ca làm:" << query.lastError().text();
            openData.rollback();
            return false;
        }
    }
    openData.commit();
    draftShifts.clear();
    return true;
}

