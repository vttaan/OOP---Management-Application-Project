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
        "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) "
        "VALUES (:id,:date,:start,:end,:status)");
    query.bindValue(":id", id);
    query.bindValue(":date", date);
    query.bindValue(":start", start);
    query.bindValue(":end", end);
    query.bindValue(":status", 0);
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
void Schedule_Model::getAcceptedSchedule(short int id, QDate monday)
{
    for (int i = 0; i < 7; i++) {
        qDeleteAll(this->shiftList[i]);
        this->shiftList[i].clear();
    }

    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);
    QDate sunday = monday.addDays(6);

    query.prepare("SELECT * FROM SHIFT WHERE idEmployee = :id AND status = 1 AND workDate BETWEEN :start AND :end");
    query.bindValue(":id", id);
    query.bindValue(":start", monday.toString("yyyy-MM-dd"));
    query.bindValue(":end", sunday.toString("yyyy-MM-dd"));

    if (!query.exec()) return;

    while (query.next()) {
        QDate date = query.value("workDate").toDate();
        Shift *newShift = new Shift(id, date,
                                    query.value("startTime").toTime(),
                                    query.value("endTime").toTime());

        int dayInWeek = monday.daysTo(date); // monday: 0, sunday: 6
        if (dayInWeek >= 0 && dayInWeek < 7) {
            this->shiftList[dayInWeek].append(newShift);
            this->numberOfShift++;
        }
    }
}

QMap<int, QMap<int, ShiftBlock*>> Schedule_Model::getManagerWeeklyGrid(QDate monday)
{
    qDeleteAll(currentWeeklyUsers);
    currentWeeklyUsers.clear();

    QMap<int, QMap<int, ShiftBlock*>> grid;
    QDate sunday = monday.addDays(6);

    // Initialize 7x14 grid
    for (int col = 0; col < 7; ++col) {
        QDate currentDate = monday.addDays(col);
        for (int row = 0; row < 14; ++row) { // 8:00 to 21:00
            QTime start(8 + row, 0);
            QTime end(8 + row + 1, 0);
            grid[col][row] = new ShiftBlock(currentDate, start, end);
        }
    }

    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);

    query.prepare("SELECT S.*, P.* FROM SHIFT S "
                  "JOIN PROFILES P ON S.idEmployee = P.idEmployee "
                  "WHERE S.status = 1 AND S.workDate BETWEEN :start AND :end");
    query.bindValue(":start", monday.toString("yyyy-MM-dd"));
    query.bindValue(":end", sunday.toString("yyyy-MM-dd"));

    if (!query.exec()) {
        qDebug() << "Failed to fetch all accepted schedule:" << query.lastError().text();
        return grid;
    }

    while (query.next()) {
        User* user = UserFactory::createContainsUser(
            query.value("role").toString(),
            query.value("idEmployee").toInt(),
            query.value("avatarPath").toString(),
            query.value("IdCitizenIdentity").toString(),
            query.value("name").toString(),
            query.value("dob").toString(),
            query.value("address").toString(),
            query.value("phoneNum").toString(),
            query.value("Gender").toString()
        );
        currentWeeklyUsers.append(user);

        QDate workDate = query.value("workDate").toDate();
        QTime startTime = query.value("startTime").toTime();
        QTime endTime = query.value("endTime").toTime();

        int col = monday.daysTo(workDate);
        if (col >= 0 && col < 7) {
            for (int row = 0; row < 14; ++row) {
                QTime slotStart(8 + row, 0);
                QTime slotEnd(8 + row + 1, 0);
                if (startTime < slotEnd && endTime > slotStart) {
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

QMap<int, QList<QString>> Schedule_Model::getWeeklySummaryStrings() const {
    QMap<int, QList<QString>> weeklyData;
    for (int col = 0; col < shiftList.size(); ++col) {
        QList<QString> labels;
        for (const Shift* s : shiftList[col]) {
            QString label = s->getStartTime().toString("HH:mm") + " - " + s->getEndTime().toString("HH:mm");
            labels.append(label);
        }
        if (!labels.isEmpty()) weeklyData.insert(col, labels);
    }
    return weeklyData;
}
