#include "Schedule_Model.h"
#include "core/UserFactory.h"

// ─── Canonical shift boundaries (shared by multiple functions) ────────────────
static const QTime SHIFT_STARTS[3] = {QTime(7, 0), QTime(12, 0), QTime(17, 0)};
static const QTime SHIFT_ENDS[3] = {QTime(12, 0), QTime(17, 0), QTime(22, 0)};

// Returns true when [sStart, sEnd) overlaps [blockStart, blockEnd)
static bool overlapsBlock(QTime sStart, QTime sEnd, QTime blockStart, QTime blockEnd)
{
    return sStart < blockEnd && sEnd > blockStart;
}

Schedule_Model::Schedule_Model() : numberOfShift(0) {}

bool Schedule_Model::checkOverlapping(short int id, QDate date, QTime start, QTime end)
{
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);
    query.prepare("SELECT * FROM SHIFT WHERE idEmployee = :id AND workDate = :date");
    query.bindValue(":id", id);
    query.bindValue(":date", date);

    if (query.exec())
    {
        while (query.next())
        {
            QTime currentStartTime = query.value("startTime").toTime();
            QTime currentEndTime = query.value("endTime").toTime();
            if (start < currentEndTime && currentStartTime < end)
                return false;
        }
    }

    for (Shift *draft : draftShifts)
    {
        if (draft->getEmployeeID() == id && draft->getDate() == date)
        {
            if (start < draft->getEndTime() && draft->getStartTime() < end)
            {
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

    Shift *newShift = new Shift(id, date, start, end);
    newShift->setShiftId(-1); // note for draftshift
    draftShifts.append(newShift);

    QDate monday = Config::getStartOfCurrentWeek(date);
    int dayInWeek = monday.daysTo(date);
    if (dayInWeek >= 0 && dayInWeek < 7)
    {
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

void Schedule_Model::getPendingSchedule(short int id, QDate monday)
{
    for (int i = 0; i < 7; i++)
    {
        qDeleteAll(this->shiftList[i]);
        this->shiftList[i].clear();
    }

    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);
    QDate sunday = monday.addDays(6);

    query.prepare("SELECT * FROM SHIFT WHERE idEmployee = :id AND status = 0 AND workDate BETWEEN :start AND :end");
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

        int dayInWeek = monday.daysTo(date);
        if (dayInWeek >= 0 && dayInWeek < 7)
        {
            this->shiftList[dayInWeek].append(newShift);
            this->numberOfShift++;
        }
    }
}

QMap<int, QList<Shift*>> Schedule_Model::getRawStaffShifts(short int id, QDate monday, int status)
{
    QMap<int, QList<Shift*>> result;
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);
    QDate sunday = monday.addDays(6);

    query.prepare("SELECT startTime, endTime, workDate FROM SHIFT WHERE idEmployee = :id AND status = :status AND workDate BETWEEN :start AND :end");
    query.bindValue(":id", id);
    query.bindValue(":status", status);
    query.bindValue(":start", monday);
    query.bindValue(":end", sunday);

    if (!query.exec()) {
        qDebug() << "Failed to fetch raw staff shifts:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        QDate workDate = query.value("workDate").toDate();
        QTime startTime = query.value("startTime").toTime();
        QTime endTime = query.value("endTime").toTime();

        int col = monday.daysTo(workDate);
        if (col >= 0 && col < 7) {
            Shift *newShift = new Shift(id, workDate, startTime, endTime);
            result[col].append(newShift);
        }
    }
    return result;
}

FullTimeScheduleGrid Schedule_Model::getFullTimeScheduleGrid(
    short int employeeId, QDate weekStart)
{
    FullTimeScheduleGrid result(
        7, QList<FullTimeShiftStatus>(3, FullTimeShiftStatus::Available));
    if (employeeId < 0 || !weekStart.isValid())
        return result;

    QSqlDatabase database = Database::getInstance()->getDbConnect();
    QDate weekEnd = weekStart.addDays(6);

    auto priority = [](FullTimeShiftStatus status)
    {
        switch (status)
        {
        case FullTimeShiftStatus::Approved: return 4;
        case FullTimeShiftStatus::Pending: return 3;
        case FullTimeShiftStatus::Declined: return 2;
        case FullTimeShiftStatus::Available: return 1;
        case FullTimeShiftStatus::StaffShortage: return 0;
        case FullTimeShiftStatus::StaffSufficient: return 0;
        }
        return 0;
    };

    QSqlQuery employeeShifts(database);
    employeeShifts.prepare(
        "SELECT workDate, startTime, endTime, status FROM SHIFT "
        "WHERE idEmployee = :id AND workDate BETWEEN :start AND :end");
    employeeShifts.bindValue(":id", employeeId);
    employeeShifts.bindValue(":start", weekStart);
    employeeShifts.bindValue(":end", weekEnd);
    if (employeeShifts.exec())
    {
        while (employeeShifts.next())
        {
            QDate date = employeeShifts.value("workDate").toDate();
            QTime start = employeeShifts.value("startTime").toTime();
            QTime end = employeeShifts.value("endTime").toTime();
            int day = weekStart.daysTo(date);
            if (day < 0 || day >= 7)
                continue;

            if (employeeShifts.value("status").toInt() == -2)
                continue;
            FullTimeShiftStatus status = FullTimeShiftStatus::Declined;
            int databaseStatus = employeeShifts.value("status").toInt();
            if (databaseStatus == 1)
                status = FullTimeShiftStatus::Approved;
            else if (databaseStatus == 0)
                status = FullTimeShiftStatus::Pending;

            for (int shift = 0; shift < 3; ++shift)
            {
                if (!overlapsBlock(start, end, SHIFT_STARTS[shift], SHIFT_ENDS[shift]))
                    continue;
                if (priority(status) > priority(result[day][shift]))
                    result[day][shift] = status;
            }
        }
    }
    else
    {
        qDebug() << "Failed to load full-time schedule:"
                 << employeeShifts.lastError().text();
        return result;
    }

    // Derive the same staffing states used by the hourly grid for cells without
    // a pending, approved, or declined registration from this employee.
    QSqlQuery profile(database);
    profile.prepare("SELECT role FROM PROFILES WHERE idEmployee = :id");
    profile.bindValue(":id", employeeId);
    QString employeeRole;
    if (profile.exec() && profile.next())
        employeeRole = profile.value("role").toString();

    int minimumForRole = Config::getMinStaffForRole(employeeRole);
    if (!employeeRole.isEmpty() && minimumForRole > 1)
    {
        QList<QList<int>> acceptedCounts(7, QList<int>(3, 0));
        QSqlQuery staffing(database);
        staffing.prepare(
            "SELECT S.workDate, S.startTime, S.endTime FROM SHIFT S "
            "JOIN PROFILES P ON P.idEmployee = S.idEmployee "
            "WHERE S.status = 1 AND P.role = :role "
            "AND S.workDate BETWEEN :start AND :end");
        staffing.bindValue(":role", employeeRole);
        staffing.bindValue(":start", weekStart);
        staffing.bindValue(":end", weekEnd);
        if (staffing.exec())
        {
            while (staffing.next())
            {
                int day = weekStart.daysTo(staffing.value(0).toDate());
                if (day < 0 || day >= 7)
                    continue;
                QTime start = staffing.value(1).toTime();
                QTime end = staffing.value(2).toTime();
                for (int shift = 0; shift < 3; ++shift)
                    if (overlapsBlock(start, end, SHIFT_STARTS[shift], SHIFT_ENDS[shift]))
                        ++acceptedCounts[day][shift];
            }

            for (int day = 0; day < 7; ++day)
            {
                for (int shift = 0; shift < 3; ++shift)
                {
                    if (result[day][shift] != FullTimeShiftStatus::Available)
                        continue;

                    if (acceptedCounts[day][shift] < minimumForRole)
                    {
                        result[day][shift] = FullTimeShiftStatus::StaffShortage;
                    }
                    else
                    {
                        result[day][shift] = FullTimeShiftStatus::StaffSufficient;
                    }
                }
            }
        }
    }

    return result;
}

QMap<int, QMap<int, ShiftBlock *>> Schedule_Model::getManagerWeeklyGrid(QDate monday, int status)
{
    qDeleteAll(currentWeeklyUsers);
    currentWeeklyUsers.clear();

    QMap<int, QMap<int, ShiftBlock *>> grid;
    QDate sunday = monday.addDays(6);

    // Initialize a 7x3 grid — one slot per canonical shift (Sáng/Chiều/Tối) per day.
    // This matches exactly what the Manager UI expects (3 rows).
    for (int col = 0; col < 7; ++col)
    {
        QDate currentDate = monday.addDays(col);
        for (int row = 0; row < 3; ++row)
        {
            grid[col][row] = new ShiftBlock(currentDate, SHIFT_STARTS[row], SHIFT_ENDS[row]);
        }
    }

    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);

    query.prepare("SELECT S.rowid AS shiftId, S.*, P.* FROM SHIFT S "
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
            query.value("Salary").toInt(),
            query.value("isFixed").toBool());
        if (!user)
            continue;
        currentWeeklyUsers.append(user);

        QDate workDate = query.value("workDate").toDate();
        QTime startTime = query.value("startTime").toTime();
        QTime endTime = query.value("endTime").toTime();

        int col = monday.daysTo(workDate);
        if (col >= 0 && col < 7)
        {
            int shiftId = query.value("shiftId").toInt();
            // Map the staff member's flexible hours into every canonical shift
            // they overlap. A shift of 09:00-14:00 will appear in both
            // Sáng (08-12) and Chiều (13-17), matching real business logic.
            for (int row = 0; row < 3; ++row)
            {
                if (overlapsBlock(startTime, endTime, SHIFT_STARTS[row], SHIFT_ENDS[row]))
                {
                    grid[col][row]->addStaff(user, shiftId);
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
    q.bindValue(":end", weekEnd.toString(Qt::ISODate));
    if (!q.exec())
        return regs;
    while (q.next())
    {
        int shiftId = q.value(0).toInt();
        int empId = q.value(1).toInt();
        QDate date = QDate::fromString(q.value(2).toString(), Qt::ISODate);
        QTime startT = QTime::fromString(q.value(3).toString(), "H:mm");
        QTime endT = QTime::fromString(q.value(4).toString(), "H:mm");
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
        q.prepare("SELECT * FROM PROFILES WHERE status != 'suspended' OR status IS NULL");
        if (q.exec())
        {
            while (q.next())
            {
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
                    q.value("Salary").toInt(),
                    q.value("isFixed").toBool());
                if (!user)
                    continue;
                currentWeeklyUsers.append(user);
                employeeMinutesWorked[user] = minutesMap.value(user->getIdEmployee(), 0);
            }
        }
    }
    return employeeMinutesWorked;
}

AutoSchedulePreview Schedule_Model::previewGeneratedSchedule(QDate weekStart)
{
    AutoSchedulePreview preview;
    if (!weekStart.isValid())
        weekStart = Config::getStartOfCurrentWeek(QDate::currentDate()).addDays(7);
    QDate weekEnd = weekStart.addDays(6);

    QVector<Shift *> pendingShifts = fetchPendingShifts(weekStart, weekEnd);
    QMap<User *, int> employeeMinutes = fetchAllEmployeeInfos(weekStart);

    Optimizer opt(pendingShifts, employeeMinutes);
    opt.solve();
    preview.warnings = opt.getWarnings();

    QHash<int, User *> usersById;
    for (User *user : currentWeeklyUsers)
        if (user) usersById.insert(user->getIdEmployee(), user);

    for (Shift *shift : pendingShifts)
    {
        if (!shift || shift->getStatus() == 0) continue;
        ManagerScheduleChange change;
        change.type = shift->getStatus() == 1
            ? ManagerScheduleChangeType::Approve
            : ManagerScheduleChangeType::Decline;
        change.shiftId = shift->getShiftId();
        change.employeeId = shift->getEmployeeID();
        change.date = shift->getDate();
        change.startTime = shift->getStartTime();
        change.endTime = shift->getEndTime();
        change.reason = "Đề xuất bởi xếp lịch tự động";
        if (User *user = usersById.value(change.employeeId, nullptr))
        {
            change.employeeName = user->getName();
            change.role = user->getRole();
        }
        preview.changes.append(change);
        if (change.type == ManagerScheduleChangeType::Approve)
            ++preview.approvedCount;
        else
            ++preview.declinedCount;
    }

    qDeleteAll(pendingShifts);
    return preview;
}

QStringList Schedule_Model::generateSchedule()
{
    AutoSchedulePreview preview = previewGeneratedSchedule(
        Config::getStartOfCurrentWeek(QDate::currentDate()).addDays(7));
    QStringList errors;
    if (!preview.changes.isEmpty() &&
        !applyManagerScheduleChanges(preview.changes, &errors))
        preview.warnings.append(errors);
    return preview.warnings;
}

bool Schedule_Model::saveDraftShiftsToDatabase()
{
    if (draftShifts.isEmpty())
        return true;
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    if (!openData.transaction())
        return false;
    QSqlQuery query(openData);
    query.prepare(
        "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status, isHoliday) "
        "VALUES (:id, :date, :start, :end, :status, :isHoliday)");
    QList<QString> holidayList = {"01/01", "30/04", "01/05", "02/09"};
    for (Shift *shift : draftShifts)
    {
        query.bindValue(":id", shift->getEmployeeID());
        query.bindValue(":date", shift->getDate());
        query.bindValue(":start", shift->getStartTime());
        query.bindValue(":end", shift->getEndTime());
        query.bindValue(":status", 0); // 0 = Chờ duyệt

        bool isHoliday = holidayList.contains(shift->getDate().toString("dd/MM"));
        query.bindValue(":isHoliday", isHoliday);

        if (!query.exec())
        {
            qDebug() << "Lỗi khi lưu ca làm:" << query.lastError().text();
            openData.rollback();
            return false;
        }
    }
    openData.commit();
    draftShifts.clear();
    return true;
}

bool Schedule_Model::replacePendingShiftsForWeek(
    short int employeeId,
    QDate weekStart,
    const QList<StaffShiftRegistration> &registrations)
{
    if (employeeId < 0 || !weekStart.isValid())
        return false;

    QDate weekEnd = weekStart.addDays(6);
    for (int i = 0; i < registrations.size(); ++i)
    {
        const StaffShiftRegistration &registration = registrations[i];
        if (!registration.date.isValid() ||
            registration.date < weekStart || registration.date > weekEnd ||
            !registration.startTime.isValid() ||
            !registration.endTime.isValid() ||
            registration.startTime >= registration.endTime)
        {
            return false;
        }

        for (int j = i + 1; j < registrations.size(); ++j)
        {
            const StaffShiftRegistration &other = registrations[j];
            if (registration.date == other.date &&
                registration.startTime < other.endTime &&
                other.startTime < registration.endTime)
            {
                return false;
            }
        }
    }

    QSqlDatabase database = Database::getInstance()->getDbConnect();
    if (!database.transaction())
        return false;

    // Approved shifts are immutable from the employee registration screen.
    // Validate the replacement before deleting any pending rows.
    QSqlQuery approvedQuery(database);
    approvedQuery.prepare(
        "SELECT startTime, endTime FROM SHIFT "
        "WHERE idEmployee = :id AND workDate = :date AND status = 1");
    for (const StaffShiftRegistration &registration : registrations)
    {
        approvedQuery.bindValue(":id", employeeId);
        approvedQuery.bindValue(":date", registration.date);
        if (!approvedQuery.exec())
        {
            database.rollback();
            return false;
        }

        while (approvedQuery.next())
        {
            QTime approvedStart = approvedQuery.value("startTime").toTime();
            QTime approvedEnd = approvedQuery.value("endTime").toTime();
            if (registration.startTime < approvedEnd &&
                approvedStart < registration.endTime)
            {
                database.rollback();
                return false;
            }
        }
    }

    auto registrationKey = [](QDate date, QTime start, QTime end)
    {
        return QString("%1|%2|%3")
            .arg(date.toString(Qt::ISODate),
                 start.toString("HH:mm:ss"),
                 end.toString("HH:mm:ss"));
    };

    QSet<QString> desiredKeys;
    for (const StaffShiftRegistration &registration : registrations)
    {
        desiredKeys.insert(registrationKey(registration.date,
                                           registration.startTime,
                                           registration.endTime));
    }

    QList<QPair<int, QString>> existingPendingRows;
    QSqlQuery existingQuery(database);
    existingQuery.prepare(
        "SELECT rowid, workDate, startTime, endTime FROM SHIFT "
        "WHERE idEmployee = :id AND status = 0 "
        "AND workDate BETWEEN :start AND :end");
    existingQuery.bindValue(":id", employeeId);
    existingQuery.bindValue(":start", weekStart);
    existingQuery.bindValue(":end", weekEnd);
    if (!existingQuery.exec())
    {
        database.rollback();
        return false;
    }
    while (existingQuery.next())
    {
        existingPendingRows.append(
            qMakePair(existingQuery.value(0).toInt(),
                      registrationKey(existingQuery.value(1).toDate(),
                                      existingQuery.value(2).toTime(),
                                      existingQuery.value(3).toTime())));
    }

    QSet<QString> retainedKeys;
    QList<int> pendingIdsToDelete;
    for (const QPair<int, QString> &existing : existingPendingRows)
    {
        if (desiredKeys.contains(existing.second) &&
            !retainedKeys.contains(existing.second))
        {
            retainedKeys.insert(existing.second);
        }
        else
        {
            pendingIdsToDelete.append(existing.first);
        }
    }

    QSqlQuery deleteQuery(database);
    deleteQuery.prepare(
        "DELETE FROM SHIFT WHERE rowid = :shiftId "
        "AND idEmployee = :id AND status = 0");
    for (int shiftId : pendingIdsToDelete)
    {
        deleteQuery.bindValue(":shiftId", shiftId);
        deleteQuery.bindValue(":id", employeeId);
        if (!deleteQuery.exec())
        {
            database.rollback();
            return false;
        }
    }

    QSqlQuery insertQuery(database);
    insertQuery.prepare(
        "INSERT INTO SHIFT "
        "(idEmployee, workDate, startTime, endTime, status, isHoliday) "
        "VALUES (:id, :date, :start, :end, 0, :isHoliday)");
    const QList<QString> holidays = {"01/01", "30/04", "01/05", "02/09"};
    for (const StaffShiftRegistration &registration : registrations)
    {
        QString key = registrationKey(registration.date,
                                      registration.startTime,
                                      registration.endTime);
        if (retainedKeys.contains(key))
            continue;

        insertQuery.bindValue(":id", employeeId);
        insertQuery.bindValue(":date", registration.date);
        insertQuery.bindValue(":start", registration.startTime);
        insertQuery.bindValue(":end", registration.endTime);
        insertQuery.bindValue(
            ":isHoliday",
            holidays.contains(registration.date.toString("dd/MM")));
        if (!insertQuery.exec())
        {
            database.rollback();
            return false;
        }
    }

    return database.commit();
}

// ─────────────────────────────────────────────────────────────────────────────
// Xếp Lịch Làm helpers
// ─────────────────────────────────────────────────────────────────────────────
// (SHIFT_STARTS, SHIFT_ENDS, overlapsBlock are defined at the top of this file)

QMap<int, QMap<int, BlockCounts>>
Schedule_Model::getAssignBlockCounts(QDate monday)
{
    QMap<int, QMap<int, BlockCounts>> result;
    // Initialise all 7x3 cells to zero explicitly to guarantee no garbage values
    for (int col = 0; col < 7; ++col)
    {
        for (int row = 0; row < 3; ++row)
        {
            BlockCounts bc;
            bc.pending = 0;
            bc.accepted = 0;
            bc.declined = 0;
            bc.required = 0;
            bc.cancelled = 0;
            for (const QString &role : Config::getAllRoles())
                bc.required += Config::getMinStaffForRole(role);
            result[col][row] = bc;
        }
    }

    QDate sunday = monday.addDays(6);
    QSqlQuery q(Database::getInstance()->getDbConnect());
    // Fetch all shifts (any status) for the target week
    q.prepare("SELECT workDate, startTime, endTime, status "
              "FROM SHIFT "
              "WHERE workDate BETWEEN :start AND :end");
    q.bindValue(":start", monday);
    q.bindValue(":end", sunday);
    if (!q.exec())
        return result;

    while (q.next())
    {
        QDate date = q.value(0).toDate();
        QTime sTime = q.value(1).toTime();
        QTime eTime = q.value(2).toTime();
        short st = static_cast<short>(q.value(3).toInt());
        int col = monday.daysTo(date);
        if (col < 0 || col >= 7)
            continue;

        for (int row = 0; row < 3; ++row)
        {
            if (!overlapsBlock(sTime, eTime, SHIFT_STARTS[row], SHIFT_ENDS[row]))
                continue;
            if (st == 0)
                result[col][row].pending++;
            else if (st == 1)
                result[col][row].accepted++;
            else if (st == -1)
                result[col][row].declined++;
            else if (st == -2)
                result[col][row].cancelled++;
        }
    }
    return result;
}

QList<EligibleEmployeeInfo> Schedule_Model::getEligibleEmployees(
    QDate date, QTime startTime, QTime endTime)
{
    QList<EligibleEmployeeInfo> result;
    if (!date.isValid() || !startTime.isValid() || !endTime.isValid() ||
        startTime >= endTime)
        return result;

    QSqlDatabase db = Database::getInstance()->getDbConnect();
    QSqlQuery profiles(db);
    profiles.prepare(
        "SELECT idEmployee, name, role, isFixed FROM PROFILES "
        "WHERE role NOT IN ('Admin') ORDER BY name");
    if (!profiles.exec())
        return result;

    while (profiles.next())
    {
        EligibleEmployeeInfo info;
        info.employeeId = profiles.value(0).toInt();
        info.employeeName = profiles.value(1).toString();
        info.role = profiles.value(2).toString();
        info.isFixedSalary = profiles.value(3).toBool();
        info.eligible = true;

        QSqlQuery conflicts(db);
        conflicts.prepare(
            "SELECT startTime, endTime FROM SHIFT "
            "WHERE idEmployee = :id AND workDate = :date AND status IN (0,1) "
            "AND startTime < :end AND endTime > :start LIMIT 1");
        conflicts.bindValue(":id", info.employeeId);
        conflicts.bindValue(":date", date);
        conflicts.bindValue(":start", startTime);
        conflicts.bindValue(":end", endTime);
        if (!conflicts.exec())
        {
            info.eligible = false;
            info.reason = "Không thể kiểm tra ca trùng";
        }
        else if (conflicts.next())
        {
            info.eligible = false;
            info.reason = QString("Đã có ca trùng (%1 - %2)")
                              .arg(conflicts.value(0).toTime().toString("HH:mm"),
                                   conflicts.value(1).toTime().toString("HH:mm"));
        }

        result.append(info);
    }
    return result;
}

QStringList Schedule_Model::validateManagerScheduleChanges(
    const QList<ManagerScheduleChange> &changes) const
{
    QStringList errors;
    QSet<int> touchedShiftIds;
    QList<ManagerScheduleChange> draftAdds;
    QSqlDatabase db = Database::getInstance()->getDbConnect();

    for (const ManagerScheduleChange &change : changes)
    {
        const QString employeeLabel = change.employeeName.isEmpty()
            ? QString("ID %1").arg(change.employeeId) : change.employeeName;
        if (change.employeeId <= 0)
        {
            errors << "Có thay đổi chứa nhân viên không hợp lệ.";
            continue;
        }

        if (change.type == ManagerScheduleChangeType::Add)
        {
            if (!change.date.isValid() || !change.startTime.isValid() ||
                !change.endTime.isValid() || change.startTime >= change.endTime)
            {
                errors << QString("%1: ngày hoặc khoảng giờ thêm vào không hợp lệ.")
                              .arg(employeeLabel);
                continue;
            }

            QSqlQuery overlap(db);
            overlap.prepare(
                "SELECT 1 FROM SHIFT WHERE idEmployee = :id AND workDate = :date "
                "AND status IN (0,1) AND startTime < :end AND endTime > :start LIMIT 1");
            overlap.bindValue(":id", change.employeeId);
            overlap.bindValue(":date", change.date);
            overlap.bindValue(":start", change.startTime);
            overlap.bindValue(":end", change.endTime);
            if (!overlap.exec())
                errors << overlap.lastError().text();
            else if (overlap.next())
                errors << QString("%1 đã có ca trùng ngày %2.")
                              .arg(employeeLabel, change.date.toString("dd/MM/yyyy"));

            for (const ManagerScheduleChange &other : draftAdds)
                if (other.employeeId == change.employeeId && other.date == change.date &&
                    other.startTime < change.endTime && other.endTime > change.startTime)
                    errors << QString("%1 có hai thay đổi nháp bị trùng ngày %2.")
                                  .arg(employeeLabel, change.date.toString("dd/MM/yyyy"));
            draftAdds.append(change);
            continue;
        }

        if (change.shiftId <= 0)
        {
            errors << QString("%1: mã ca làm không hợp lệ.").arg(employeeLabel);
            continue;
        }
        if (touchedShiftIds.contains(change.shiftId))
        {
            errors << QString("Ca %1 có nhiều hành động mâu thuẫn trong bản nháp.")
                          .arg(change.shiftId);
            continue;
        }
        touchedShiftIds.insert(change.shiftId);

        QSqlQuery exists(db);
        exists.prepare("SELECT status FROM SHIFT WHERE rowid = :id");
        exists.bindValue(":id", change.shiftId);
        if (!exists.exec())
            errors << exists.lastError().text();
        else if (!exists.next())
            errors << QString("Ca %1 không còn tồn tại.").arg(change.shiftId);
        else
        {
            int status = exists.value(0).toInt();
            if (status != 0 && status != 1)
                errors << QString("Ca %1 đã được xử lý bởi thay đổi khác.")
                              .arg(change.shiftId);
        }
    }

    errors.removeDuplicates();
    return errors;
}

bool Schedule_Model::applyManagerScheduleChanges(
    const QList<ManagerScheduleChange> &changes, QStringList *errors)
{
    if (changes.isEmpty())
        return true;

    QStringList validationErrors = validateManagerScheduleChanges(changes);
    if (!validationErrors.isEmpty())
    {
        if (errors) errors->append(validationErrors);
        return false;
    }

    QSqlDatabase db = Database::getInstance()->getDbConnect();
    if (!db.transaction())
    {
        if (errors) errors->append("Không thể bắt đầu giao dịch cập nhật lịch.");
        return false;
    }

    QSqlQuery createAudit(db);
    if (!createAudit.exec(
            "CREATE TABLE IF NOT EXISTS SHIFT_AUDIT ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, shiftId INTEGER, "
            "employeeId INTEGER NOT NULL, action TEXT NOT NULL, reason TEXT, "
            "changedAt TEXT NOT NULL)"))
    {
        db.rollback();
        if (errors) errors->append(createAudit.lastError().text());
        return false;
    }

    auto fail = [&](const QString &message) {
        db.rollback();
        if (errors) errors->append(message);
        return false;
    };

    for (const ManagerScheduleChange &change : changes)
    {
        if (change.employeeId <= 0)
            return fail("Nhân viên không hợp lệ.");

        if (change.type == ManagerScheduleChangeType::Add)
        {
            if (!change.date.isValid())
                return fail("Ngày làm việc không hợp lệ.");
            if (!change.startTime.isValid() || !change.endTime.isValid() ||
                change.startTime >= change.endTime)
                return fail("Khoảng thời gian thêm vào không hợp lệ.");

            QSqlQuery overlap(db);
            overlap.prepare(
                "SELECT 1 FROM SHIFT WHERE idEmployee = :id AND workDate = :date "
                "AND status IN (0,1) AND startTime < :end AND endTime > :start LIMIT 1");
            overlap.bindValue(":id", change.employeeId);
            overlap.bindValue(":date", change.date);
            overlap.bindValue(":start", change.startTime);
            overlap.bindValue(":end", change.endTime);
            if (!overlap.exec())
                return fail(overlap.lastError().text());
            if (overlap.next())
                return fail(QString("Nhân viên %1 đã có ca trùng.").arg(change.employeeId));

            QSqlQuery insert(db);
            insert.prepare(
                "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status, isHoliday) "
                "VALUES (:id, :date, :start, :end, 1, 0)");
            insert.bindValue(":id", change.employeeId);
            insert.bindValue(":date", change.date);
            insert.bindValue(":start", change.startTime);
            insert.bindValue(":end", change.endTime);
            if (!insert.exec())
                return fail(insert.lastError().text());

            QSqlQuery audit(db);
            audit.prepare("INSERT INTO SHIFT_AUDIT (shiftId, employeeId, action, reason, changedAt) "
                          "VALUES (:shift, :employee, 'add', :reason, :at)");
            audit.bindValue(":shift", insert.lastInsertId());
            audit.bindValue(":employee", change.employeeId);
            audit.bindValue(":reason", change.reason);
            audit.bindValue(":at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
            if (!audit.exec()) return fail(audit.lastError().text());
        }
        else
        {
            if (change.shiftId <= 0)
                return fail("Mã ca làm không hợp lệ.");

            int targetStatus = change.type == ManagerScheduleChangeType::Approve ? 1
                              : change.type == ManagerScheduleChangeType::Decline ? -1
                              : -2;
            QSqlQuery update(db);
            update.prepare("UPDATE SHIFT SET status = :status WHERE rowid = :id "
                           "AND status IN (0,1)");
            update.bindValue(":status", targetStatus);
            update.bindValue(":id", change.shiftId);
            if (!update.exec() || update.numRowsAffected() != 1)
                return fail(QString("Không thể cập nhật ca %1.").arg(change.shiftId));

            QSqlQuery audit(db);
            audit.prepare("INSERT INTO SHIFT_AUDIT (shiftId, employeeId, action, reason, changedAt) "
                          "VALUES (:shift, :employee, :action, :reason, :at)");
            audit.bindValue(":shift", change.shiftId);
            audit.bindValue(":employee", change.employeeId);
            audit.bindValue(":action", targetStatus == 1 ? "approve" :
                                         targetStatus == -1 ? "decline" : "cancel");
            audit.bindValue(":reason", change.reason);
            audit.bindValue(":at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
            if (!audit.exec()) return fail(audit.lastError().text());
        }
    }

    return db.commit();
}

QList<PendingShiftInfo> Schedule_Model::getShiftsForBlock(QDate monday, int col, int row)
{
    QList<PendingShiftInfo> list;
    if (col < 0 || col >= 7 || row < 0 || row >= 3)
        return list;

    QDate targetDate = monday.addDays(col);
    QTime bStart = SHIFT_STARTS[row];
    QTime bEnd = SHIFT_ENDS[row];

    QSqlQuery q(Database::getInstance()->getDbConnect());
    q.prepare("SELECT S.rowid, S.idEmployee, S.startTime, S.endTime, S.status, "
              "       P.name, P.role "
              "FROM SHIFT S "
              "JOIN PROFILES P ON S.idEmployee = P.idEmployee "
                  "WHERE S.workDate = :date AND S.status <> -2");
    q.bindValue(":date", targetDate);
    if (!q.exec())
        return list;

    while (q.next())
    {
        QTime sTime = q.value(2).toTime();
        QTime eTime = q.value(3).toTime();
        if (!overlapsBlock(sTime, eTime, bStart, bEnd))
            continue;

        PendingShiftInfo info;
        info.shiftId = q.value(0).toInt();
        info.employeeId = q.value(1).toInt();
        info.date = targetDate;
        info.startTime = sTime;
        info.endTime = eTime;
        info.status = static_cast<short>(q.value(4).toInt());
        info.employeeName = q.value(5).toString();
        info.role = q.value(6).toString();
        list.append(info);
    }

    // Sort: pending first, then accepted, then declined (for popup table ordering)
    std::sort(list.begin(), list.end(), [](const PendingShiftInfo &a, const PendingShiftInfo &b)
              {
        auto rank = [](short s) { return (s == 0) ? 0 : (s == 1) ? 1 : 2; };
        return rank(a.status) < rank(b.status); });

    return list;
}

bool Schedule_Model::approveShift(int shiftId)
{
    QSqlQuery q(Database::getInstance()->getDbConnect());
    q.prepare("UPDATE SHIFT SET status = 1 WHERE rowid = :id");
    q.bindValue(":id", shiftId);
    return q.exec();
}

bool Schedule_Model::declineShift(int shiftId)
{
    QSqlQuery q(Database::getInstance()->getDbConnect());
    q.prepare("UPDATE SHIFT SET status = -1 WHERE rowid = :id");
    q.bindValue(":id", shiftId);
    return q.exec();
}

QList<PendingShiftInfo> Schedule_Model::getEligibleReplacements(int oldShiftId, const QString &role)
{
    QList<PendingShiftInfo> list;

    // 1. Get original shift details
    QSqlQuery q(Database::getInstance()->getDbConnect());
    q.prepare("SELECT workDate, startTime, endTime FROM SHIFT WHERE rowid = :id");
    q.bindValue(":id", oldShiftId);
    if (!q.exec() || !q.next())
        return list;

    QDate workDate = q.value(0).toDate();
    QTime startTime = q.value(1).toTime();
    QTime endTime = q.value(2).toTime();

    // 2. Find eligible replacements (status = -1 as requested)
    QSqlQuery rq(Database::getInstance()->getDbConnect());
    rq.prepare("SELECT S.rowid, S.idEmployee, S.startTime, S.endTime, S.status, "
               "       P.name, P.role "
               "FROM SHIFT S "
               "JOIN PROFILES P ON S.idEmployee = P.idEmployee "
               "WHERE S.workDate = :date AND S.status = -1 AND P.role = :role");
    rq.bindValue(":date", workDate);
    rq.bindValue(":role", role);
    if (!rq.exec())
        return list;

    while (rq.next())
    {
        QTime sTime = rq.value(2).toTime();
        QTime eTime = rq.value(3).toTime();
        
        // We consider it a replacement if the pending shift overlaps the original shift's block
        if (!overlapsBlock(sTime, eTime, startTime, endTime))
            continue;

        PendingShiftInfo info;
        info.shiftId = rq.value(0).toInt();
        info.employeeId = rq.value(1).toInt();
        info.startTime = sTime;
        info.endTime = eTime;
        info.status = static_cast<short>(rq.value(4).toInt());
        info.employeeName = rq.value(5).toString();
        info.role = rq.value(6).toString();
        list.append(info);
    }

    return list;
}

bool Schedule_Model::replaceShift(int oldShiftId, int newShiftId)
{
    auto db = Database::getInstance()->getDbConnect();
    db.transaction();
    
    QSqlQuery q1(db);
    q1.prepare("UPDATE SHIFT SET status = -1 WHERE rowid = :id");
    q1.bindValue(":id", oldShiftId);
    if (!q1.exec()) {
        db.rollback();
        return false;
    }
    
    QSqlQuery q2(db);
    q2.prepare("UPDATE SHIFT SET status = 1 WHERE rowid = :id");
    q2.bindValue(":id", newShiftId);
    if (!q2.exec()) {
        db.rollback();
        return false;
    }
    
    return db.commit();
}
