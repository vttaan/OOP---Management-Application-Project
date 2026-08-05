#include "Schedule_Model.h"
#include "core/UserFactory.h"
#include "model/Notification_Model.h"

// ─── Canonical shift boundaries (shared by multiple functions) ────────────────
static const QTime SHIFT_STARTS[3] = {QTime(7, 0), QTime(12, 0), QTime(17, 0)};
static const QTime SHIFT_ENDS[3] = {QTime(12, 0), QTime(17, 0), QTime(22, 0)};

// Returns true when [sStart, sEnd) overlaps [blockStart, blockEnd)
static bool overlapsBlock(QTime sStart, QTime sEnd, QTime blockStart, QTime blockEnd)
{
    return sStart < blockEnd && sEnd > blockStart;
}

static QTime databaseTime(const QVariant &value)
{
    const QTime converted = value.toTime();
    if (converted.isValid())
        return converted;

    const QString text = value.toString().trimmed();
    static const QStringList formats = {
        "HH:mm:ss.zzz", "H:mm:ss.zzz", "HH:mm:ss", "H:mm:ss",
        "HH:mm", "H:mm"};
    for (const QString &format : formats)
    {
        const QTime parsed = QTime::fromString(text, format);
        if (parsed.isValid())
            return parsed;
    }
    return {};
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
            QTime currentStartTime = databaseTime(query.value("startTime"));
            QTime currentEndTime = databaseTime(query.value("endTime"));
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
                                    databaseTime(query.value("startTime")),
                                    databaseTime(query.value("endTime")));
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
                                    databaseTime(query.value("startTime")),
                                    databaseTime(query.value("endTime")));

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
                                    databaseTime(query.value("startTime")),
                                    databaseTime(query.value("endTime")));

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
        QTime startTime = databaseTime(query.value("startTime"));
        QTime endTime = databaseTime(query.value("endTime"));

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
            QTime start = databaseTime(employeeShifts.value("startTime"));
            QTime end = databaseTime(employeeShifts.value("endTime"));
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
                QTime start = databaseTime(staffing.value(1));
                QTime end = databaseTime(staffing.value(2));
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
        QTime startTime = databaseTime(query.value("startTime"));
        QTime endTime = databaseTime(query.value("endTime"));

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
        QTime startT = databaseTime(q.value(3));
        QTime endT = databaseTime(q.value(4));
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
                QTime start = databaseTime(q.value(1));
                QTime end = databaseTime(q.value(2));
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
        weekStart = Config::getStartOfNextWeek(QDate::currentDate());
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
        if (change.type == ManagerScheduleChangeType::Approve)
        {
            const QTime assignedStart = shift->getAssignedStartTime();
            const QTime assignedEnd = shift->getAssignedEndTime();
            if (assignedStart.isValid() && assignedEnd.isValid() &&
                assignedStart < assignedEnd)
            {
                change.startTime = assignedStart;
                change.endTime = assignedEnd;
            }
        }
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
        Config::getStartOfNextWeek(QDate::currentDate()));
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

bool Schedule_Model::ensurePendingCarryForwardForWeek(
    QDate weekStart, QStringList *errors)
{
    if (!weekStart.isValid())
    {
        if (errors) errors->append("Invalid target week.");
        return false;
    }

    QSqlDatabase database = Database::getInstance()->getDbConnect();
    if (!database.transaction())
    {
        if (errors) errors->append(database.lastError().text());
        return false;
    }

    auto fail = [&](const QString &message) {
        database.rollback();
        if (errors) errors->append(message);
        return false;
    };

    QSqlQuery ensureMarkerTable(database);
    if (!ensureMarkerTable.exec(
            "CREATE TABLE IF NOT EXISTS SHIFT_CARRY_FORWARD ("
            "idEmployee INTEGER NOT NULL, targetWeekStart TEXT NOT NULL, "
            "createdAt TEXT NOT NULL, "
            "PRIMARY KEY (idEmployee, targetWeekStart))"))
        return fail(ensureMarkerTable.lastError().text());

    QSqlQuery employees(database);
    if (!employees.exec(
            "SELECT idEmployee FROM PROFILES "
            "WHERE role NOT IN ('Manager', 'Manage', 'Admin') "
            "AND (status IS NULL OR status = 'active') "
            "ORDER BY idEmployee"))
        return fail(employees.lastError().text());

    QList<int> employeeIds;
    while (employees.next())
        employeeIds.append(employees.value(0).toInt());

    const QDate previousWeekStart = weekStart.addDays(-7);
    const QDate previousWeekEnd = weekStart.addDays(-1);
    const QList<QString> holidays = {"01/01", "30/04", "01/05", "02/09"};

    for (int employeeId : employeeIds)
    {
        QSqlQuery marker(database);
        marker.prepare(
            "SELECT 1 FROM SHIFT_CARRY_FORWARD "
            "WHERE idEmployee = :id AND targetWeekStart = :week LIMIT 1");
        marker.bindValue(":id", employeeId);
        marker.bindValue(":week", weekStart);
        if (!marker.exec())
            return fail(marker.lastError().text());
        if (marker.next())
            continue;

        QSqlQuery previousShifts(database);
        previousShifts.prepare(
            "SELECT workDate, startTime, endTime FROM SHIFT "
            "WHERE idEmployee = :id AND status = 1 "
            "AND workDate BETWEEN :start AND :end "
            "ORDER BY workDate, startTime");
        previousShifts.bindValue(":id", employeeId);
        previousShifts.bindValue(":start", previousWeekStart);
        previousShifts.bindValue(":end", previousWeekEnd);
        if (!previousShifts.exec())
            return fail(previousShifts.lastError().text());

        struct PreviousShift
        {
            QDate date;
            QTime start;
            QTime end;
        };
        QList<PreviousShift> approvedShifts;
        while (previousShifts.next())
        {
            const QDate date = previousShifts.value(0).toDate();
            const QTime start = databaseTime(previousShifts.value(1));
            const QTime end = databaseTime(previousShifts.value(2));
            if (date.isValid() && start.isValid() && end.isValid() && start < end)
                approvedShifts.append({date, start, end});
        }

        for (const PreviousShift &source : approvedShifts)
        {
            const QDate targetDate = source.date.addDays(7);

            QSqlQuery approvedLeave(database);
            approvedLeave.prepare(
                "SELECT 1 FROM LEAVE_REQUEST WHERE idEmployee = :id "
                "AND leaveDate = :date AND status = 'Approved' LIMIT 1");
            approvedLeave.bindValue(":id", employeeId);
            approvedLeave.bindValue(":date", targetDate);
            if (!approvedLeave.exec())
                return fail(approvedLeave.lastError().text());
            if (approvedLeave.next())
                continue;

            QSqlQuery overlap(database);
            overlap.prepare(
                "SELECT 1 FROM SHIFT WHERE idEmployee = :id "
                "AND workDate = :date AND status IN (0,1) "
                "AND startTime < :end AND endTime > :start LIMIT 1");
            overlap.bindValue(":id", employeeId);
            overlap.bindValue(":date", targetDate);
            overlap.bindValue(":start", source.start);
            overlap.bindValue(":end", source.end);
            if (!overlap.exec())
                return fail(overlap.lastError().text());
            if (overlap.next())
                continue;

            QSqlQuery insert(database);
            insert.prepare(
                "INSERT INTO SHIFT "
                "(idEmployee, workDate, startTime, endTime, status, isHoliday) "
                "VALUES (:id, :date, :start, :end, 0, :isHoliday)");
            insert.bindValue(":id", employeeId);
            insert.bindValue(":date", targetDate);
            insert.bindValue(":start", source.start);
            insert.bindValue(":end", source.end);
            insert.bindValue(":isHoliday",
                             holidays.contains(targetDate.toString("dd/MM")));
            if (!insert.exec())
                return fail(insert.lastError().text());
        }

        QSqlQuery markComplete(database);
        markComplete.prepare(
            "INSERT INTO SHIFT_CARRY_FORWARD "
            "(idEmployee, targetWeekStart, createdAt) VALUES (:id, :week, :at)");
        markComplete.bindValue(":id", employeeId);
        markComplete.bindValue(":week", weekStart);
        markComplete.bindValue(
            ":at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
        if (!markComplete.exec())
            return fail(markComplete.lastError().text());
    }

    if (!database.commit())
    {
        if (errors) errors->append(database.lastError().text());
        return false;
    }
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
            QTime approvedStart = databaseTime(approvedQuery.value("startTime"));
            QTime approvedEnd = databaseTime(approvedQuery.value("endTime"));
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
                                      databaseTime(existingQuery.value(2)),
                                      databaseTime(existingQuery.value(3)))));
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
    const QList<int> managerRecipients =
        Notification_Model::getManagerRecipientIds(database);
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

        const int shiftId = insertQuery.lastInsertId().toInt();
        const QString message = QString::fromUtf8(
            "Nhân viên #%1 đăng ký ca %2-%3 ngày %4.")
            .arg(employeeId,
                 registration.startTime.toString("HH:mm"),
                 registration.endTime.toString("HH:mm"),
                 registration.date.toString("dd/MM/yyyy"));
        for (int managerId : managerRecipients)
        {
            if (!Notification_Model::create(
                    database, managerId, "SHIFT_SUBMITTED",
                    QString::fromUtf8("Yêu cầu đăng ký ca mới"), message, shiftId))
            {
                database.rollback();
                return false;
            }
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
        QTime sTime = databaseTime(q.value(1));
        QTime eTime = databaseTime(q.value(2));
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

void Schedule_Model::publishStaffingWarningNotifications(QDate monday)
{
    if (!monday.isValid())
        return;

    QSqlDatabase database = Database::getInstance()->getDbConnect();
    const QList<int> managerIds = Notification_Model::getManagerRecipientIds(database);
    if (managerIds.isEmpty())
        return;

    const QMap<int, QMap<int, BlockCounts>> counts = getAssignBlockCounts(monday);
    const QStringList shiftNames = {
        QString::fromUtf8("Ca sáng"),
        QString::fromUtf8("Ca chiều"),
        QString::fromUtf8("Ca tối")};
    static const QTime shiftStarts[3] = {QTime(7, 0), QTime(12, 0), QTime(17, 0)};
    static const QTime shiftEnds[3] = {QTime(12, 0), QTime(17, 0), QTime(22, 0)};
    QSet<QString> activeWarningKeys;

    const QDate today = QDate::currentDate();
    const QTime now = QTime::currentTime();
    for (int day = 0; day < 7; ++day)
    {
        const QDate date = monday.addDays(day);
        if (date < today)
            continue;

        for (int shift = 0; shift < 3; ++shift)
        {
            if (date == today && shiftEnds[shift] <= now)
                continue;

            const BlockCounts block = counts.value(day).value(shift);
            const int deficit = qMax(0, block.required - block.accepted);
            if (deficit <= 0)
                continue;

            const double deficitRatio = block.required > 0
                ? static_cast<double>(deficit) / block.required : 0.0;
            QString severity;
            int priority = 0;
            if (deficitRatio >= 0.75)
            {
                severity = QString::fromUtf8("Khẩn cấp");
                priority = 100;
            }
            else if (deficitRatio >= 0.4)
            {
                severity = QString::fromUtf8("Thiếu nhiều");
                priority = 80;
            }
            else
            {
                severity = QString::fromUtf8("Cần bổ sung");
                priority = 60;
            }

            const QString shiftName = shiftNames.value(shift);
            const QString title = QString::fromUtf8("Cảnh báo thiếu nhân sự: %1")
                                      .arg(severity);
            const QString message = QString::fromUtf8(
                "%1 %2: đã xếp %3/%4 nhân viên, còn thiếu %5. Có %6 yêu cầu chờ duyệt.")
                .arg(date.toString("dd/MM/yyyy"), shiftName,
                     QString::number(block.accepted), QString::number(block.required),
                     QString::number(deficit), QString::number(block.pending));
            const QString dedupeKey = QString("STAFFING_SHORTAGE|%1|%2|%3|%4|%5|%6|%7")
                .arg(monday.toString(Qt::ISODate), QString::number(shift),
                     date.toString(Qt::ISODate),
                     QString::number(block.required), QString::number(block.accepted),
                     QString::number(block.pending), severity);
            activeWarningKeys.insert(dedupeKey);

            for (const int managerId : managerIds)
            {
                Notification_Model::createIfAbsent(
                    database, managerId, "STAFFING_SHORTAGE", title, message,
                    priority, dedupeKey);
            }
        }
    }

    // A warning that no longer appears in the current coverage calculation is
    // retained as history but removed from the unread queue.
    const QString weekPrefix = QString("STAFFING_SHORTAGE|%1|")
        .arg(monday.toString(Qt::ISODate));
    for (const int managerId : managerIds)
    {
        QSqlQuery stale(database);
        stale.prepare("SELECT id, dedupeKey FROM NOTIFICATION "
                      "WHERE recipientEmployeeId = :recipient "
                      "AND type = 'STAFFING_SHORTAGE' AND dedupeKey LIKE :prefix");
        stale.bindValue(":recipient", managerId);
        stale.bindValue(":prefix", weekPrefix + "%");
        if (!stale.exec())
            continue;
        while (stale.next())
        {
            if (activeWarningKeys.contains(stale.value(1).toString()))
                continue;
            QSqlQuery resolve(database);
            resolve.prepare("UPDATE NOTIFICATION SET status = 'Read', readAt = :at "
                            "WHERE id = :id AND status = 'Unread'");
            resolve.bindValue(":at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
            resolve.bindValue(":id", stale.value(0));
            resolve.exec();
        }
    }
}

void Schedule_Model::publishScheduledStaffingWarningNotifications(QDate today,
                                                                   QTime now)
{
    if (!today.isValid() || !now.isValid())
        return;

    QSqlDatabase database = Database::getInstance()->getDbConnect();
    const QList<int> managerIds = Notification_Model::getManagerRecipientIds(database);
    if (managerIds.isEmpty())
        return;

    static const QTime shiftStarts[3] = {QTime(7, 0), QTime(12, 0), QTime(17, 0)};
    static const QTime shiftEnds[3] = {QTime(12, 0), QTime(17, 0), QTime(22, 0)};
    const QStringList shiftNames = {
        QString::fromUtf8("Ca sáng"),
        QString::fromUtf8("Ca chiều"),
        QString::fromUtf8("Ca tối")};

    auto publishBlockWarning = [&](const QDate &date, int shift,
                                   const BlockCounts &block,
                                   const QString &stage) {
        const int deficit = qMax(0, block.required - block.accepted);
        if (deficit <= 0)
            return;

        const double deficitRatio = block.required > 0
            ? static_cast<double>(deficit) / block.required : 0.0;
        QString severity;
        int priority = 0;
        if (deficitRatio >= 0.75)
        {
            severity = QString::fromUtf8("Khẩn cấp");
            priority = 100;
        }
        else if (deficitRatio >= 0.4)
        {
            severity = QString::fromUtf8("Thiếu nhiều");
            priority = 80;
        }
        else
        {
            severity = QString::fromUtf8("Cần bổ sung");
            priority = 60;
        }

        const bool nextShift = stage == "SHIFT_3H";
        if (nextShift)
            priority += 100;
        const QString title = nextShift
            ? QString::fromUtf8("Cảnh báo ca sắp tới: %1").arg(severity)
            : QString::fromUtf8("Cảnh báo thiếu nhân sự ngày mai: %1").arg(severity);
        const QString message = nextShift
            ? QString::fromUtf8("%1 %2 bắt đầu lúc %3: đã xếp %4/%5 nhân viên, còn thiếu %6.")
                  .arg(date.toString("dd/MM/yyyy"), shiftNames.value(shift),
                       shiftStarts[shift].toString("HH:mm"),
                       QString::number(block.accepted), QString::number(block.required),
                       QString::number(deficit))
            : QString::fromUtf8("%1 %2: đã xếp %3/%4 nhân viên, còn thiếu %5. Có %6 yêu cầu chờ duyệt.")
                  .arg(date.toString("dd/MM/yyyy"), shiftNames.value(shift),
                       QString::number(block.accepted), QString::number(block.required),
                       QString::number(deficit), QString::number(block.pending));
        const QString dedupeKey = QString("STAFFING_SHORTAGE|%1|%2|%3|%4|%5|%6|%7")
            .arg(date.toString(Qt::ISODate), QString::number(shift), stage,
                 QString::number(block.required), QString::number(block.accepted),
                 QString::number(block.pending), severity);

        for (const int managerId : managerIds)
            Notification_Model::createIfAbsent(
                database, managerId, "STAFFING_SHORTAGE", title, message,
                priority, dedupeKey);
    };

    // One day before: publish every shortage in tomorrow's three shift blocks.
    const QDate tomorrow = today.addDays(1);
    const QDate tomorrowWeek = Config::getStartOfCurrentWeek(tomorrow);
    const QMap<int, QMap<int, BlockCounts>> tomorrowCounts =
        getAssignBlockCounts(tomorrowWeek);
    const int tomorrowColumn = tomorrowWeek.daysTo(tomorrow);
    if (tomorrowColumn >= 0 && tomorrowColumn < 7)
    {
        for (int shift = 0; shift < 3; ++shift)
            publishBlockWarning(tomorrow, shift,
                                tomorrowCounts.value(tomorrowColumn).value(shift),
                                "DAY_BEFORE");
    }

    // Day of work: publish only the next upcoming shift when it is within
    // three hours. The timer may run late, so the whole [0, 3h] window is valid.
    int nextShift = -1;
    for (int shift = 0; shift < 3; ++shift)
    {
        if (now < shiftStarts[shift])
        {
            nextShift = shift;
            break;
        }
    }
    if (nextShift < 0)
        return;

    const int secondsUntilNextShift = now.secsTo(shiftStarts[nextShift]);
    if (secondsUntilNextShift <= 0 || secondsUntilNextShift > 3 * 60 * 60)
        return;

    const QDate todayWeek = Config::getStartOfCurrentWeek(today);
    const int todayColumn = todayWeek.daysTo(today);
    const QMap<int, QMap<int, BlockCounts>> todayCounts =
        getAssignBlockCounts(todayWeek);
    if (todayColumn >= 0 && todayColumn < 7)
        publishBlockWarning(today, nextShift,
                            todayCounts.value(todayColumn).value(nextShift),
                            "SHIFT_3H");
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
        "SELECT idEmployee, name, role, isFixed, status FROM PROFILES "
        "WHERE role NOT IN ('Admin', 'Manager', 'Manage') ORDER BY name");
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
        const QString profileStatus = profiles.value(4).toString().trimmed();
        if (!profileStatus.isEmpty() &&
            profileStatus.compare("active", Qt::CaseInsensitive) != 0)
        {
            info.eligible = false;
            info.reason = QString::fromUtf8("Nhân viên hiện không hoạt động.");
        }

        QSqlQuery leave(db);
        leave.prepare("SELECT 1 FROM LEAVE_REQUEST WHERE idEmployee = :id "
                      "AND leaveDate = :date AND status = 'Approved' LIMIT 1");
        leave.bindValue(":id", info.employeeId);
        leave.bindValue(":date", date);
        if (!leave.exec())
        {
            info.eligible = false;
            info.reason = QString::fromUtf8("Không thể kiểm tra nghỉ phép.");
        }
        else if (leave.next())
        {
            info.eligible = false;
            info.reason = QString::fromUtf8("Đã được duyệt nghỉ phép.");
        }

        QSqlQuery conflicts(db);
        conflicts.prepare(
            "SELECT startTime, endTime FROM SHIFT "
            "WHERE idEmployee = :id AND workDate = :date AND status IN (0,1) "
            "AND startTime < :end AND endTime > :start LIMIT 1");
        conflicts.bindValue(":id", info.employeeId);
        conflicts.bindValue(":date", date);
        conflicts.bindValue(":start", startTime);
        conflicts.bindValue(":end", endTime);
        if (!info.eligible)
        {
            result.append(info);
            continue;
        }
        if (!conflicts.exec())
        {
            info.eligible = false;
            info.reason = "Không thể kiểm tra ca trùng";
        }
        else if (conflicts.next())
        {
            info.eligible = false;
            info.reason = QString("Đã có ca trùng (%1 - %2)")
                              .arg(databaseTime(conflicts.value(0)).toString("HH:mm"),
                                   databaseTime(conflicts.value(1)).toString("HH:mm"));
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

        QSqlQuery profile(db);
        profile.prepare("SELECT role FROM PROFILES WHERE idEmployee = :employee");
        profile.bindValue(":employee", change.employeeId);
        if (!profile.exec())
        {
            errors << profile.lastError().text();
            continue;
        }
        if (!profile.next())
        {
            errors << QString("%1 không còn tồn tại.").arg(employeeLabel);
            continue;
        }
        const QString role = profile.value(0).toString();
        if (role.compare("Manager", Qt::CaseInsensitive) == 0 ||
            role.compare("Manage", Qt::CaseInsensitive) == 0 ||
            role.compare("Admin", Qt::CaseInsensitive) == 0)
        {
            errors << QString("%1 không thuộc nhóm nhân viên được phân ca.")
                          .arg(employeeLabel);
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

            QSqlQuery approvedLeave(db);
            approvedLeave.prepare(
                "SELECT 1 FROM LEAVE_REQUEST WHERE idEmployee = :id "
                "AND leaveDate = :date AND status = 'Approved' LIMIT 1");
            approvedLeave.bindValue(":id", change.employeeId);
            approvedLeave.bindValue(":date", change.date);
            if (!approvedLeave.exec())
            {
                errors << approvedLeave.lastError().text();
                continue;
            }
            if (approvedLeave.next())
            {
                errors << QString("%1 Ä‘Ã£ Ä‘Æ°á»£c duyá»‡t nghá»‰ phÃ©p ngÃ y %2.")
                              .arg(employeeLabel, change.date.toString("dd/MM/yyyy"));
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

        if (change.type == ManagerScheduleChangeType::Approve &&
            (!change.date.isValid() || !change.startTime.isValid() ||
             !change.endTime.isValid() || change.startTime >= change.endTime))
        {
            errors << QString("%1: ngày hoặc khoảng giờ duyệt không hợp lệ.")
                          .arg(employeeLabel);
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

            const int shiftId = insert.lastInsertId().toInt();
            if (!Notification_Model::create(
                    db, change.employeeId, "SHIFT_ADDED",
                    QString::fromUtf8("Bạn có ca làm mới"),
                    QString::fromUtf8("Bạn được phân ca %1-%2 ngày %3.%4")
                        .arg(change.startTime.toString("HH:mm"),
                             change.endTime.toString("HH:mm"),
                             change.date.toString("dd/MM/yyyy"),
                             change.reason.trimmed().isEmpty()
                                 ? QString()
                                 : QString::fromUtf8(" Lý do: %1").arg(change.reason.trimmed())),
                    shiftId))
                return fail(QString::fromUtf8("Không thể tạo thông báo phân ca."));
        }
        else
        {
            if (change.shiftId <= 0)
                return fail("Mã ca làm không hợp lệ.");

            int targetStatus = change.type == ManagerScheduleChangeType::Approve ? 1
                              : change.type == ManagerScheduleChangeType::Decline ? -1
                              : -2;
            QSqlQuery update(db);
            if (targetStatus == 1)
            {
                update.prepare(
                    "UPDATE SHIFT SET status = :status, startTime = :start, "
                    "endTime = :end WHERE rowid = :id AND status IN (0,1)");
                update.bindValue(":start", change.startTime);
                update.bindValue(":end", change.endTime);
            }
            else
            {
                update.prepare("UPDATE SHIFT SET status = :status WHERE rowid = :id "
                               "AND status IN (0,1)");
            }
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

            const QString type = targetStatus == 1 ? "SHIFT_APPROVED"
                               : targetStatus == -1 ? "SHIFT_DECLINED"
                                                    : "SHIFT_CANCELLED";
            const QString title = targetStatus == 1
                ? QString::fromUtf8("Yêu cầu ca làm đã được duyệt")
                : targetStatus == -1
                    ? QString::fromUtf8("Yêu cầu ca làm bị từ chối")
                    : QString::fromUtf8("Ca làm đã bị hủy");
            const QString message = QString::fromUtf8("Ca %1-%2 ngày %3.%4")
                .arg(change.startTime.toString("HH:mm"),
                     change.endTime.toString("HH:mm"),
                     change.date.toString("dd/MM/yyyy"),
                     change.reason.trimmed().isEmpty()
                         ? QString()
                         : QString::fromUtf8(" Lý do: %1").arg(change.reason.trimmed()));
            if (!Notification_Model::create(db, change.employeeId, type, title,
                                            message, change.shiftId))
                return fail(QString::fromUtf8("Không thể tạo thông báo cập nhật ca."));
        }
    }

    if (!db.commit())
        return false;

    // Publish the same live staffing warnings immediately after a manager
    // changes coverage. Notification_Control also refreshes them when the
    // inbox opens, while dedupe keys prevent duplicate rows.
    publishScheduledStaffingWarningNotifications(QDate::currentDate(),
                                                 QTime::currentTime());
    return true;
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
        QTime sTime = databaseTime(q.value(2));
        QTime eTime = databaseTime(q.value(3));
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
