#include "Schedule_Model.h"

Schedule_Model::Schedule_Model() : numberOfShift(0) {}


bool Schedule_Model::checkOverlapping(short int id, QDate date, QTime start, QTime end) {
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);
    query.prepare("SELECT * FROM SHIFT WHERE idEmployee = :id AND workDate = :date");
    query.bindValue(":id", id);
    query.bindValue(":date", date);

    if (!query.exec()) return true;
    while (query.next()) {
        QTime currentStartTime = QTime::fromString(query.value("startTime").toString(), "hh:mm");
        QTime currentEndTime = QTime::fromString(query.value("endTime").toString(), "hh:mm");
        if ((start < currentStartTime && end > currentStartTime) ||
            (start < currentEndTime  && end > currentEndTime) ||
            (start < currentStartTime && end > currentEndTime) ||
            (start > currentStartTime && end < currentEndTime)) return false;
    }
    return true;
}

Shift* Schedule_Model::getPreviewShift(short int id, QDate date, QTime start, QTime end) {
    return new Shift(id, date, start, end);
}

pair<QDate, QDate> Schedule_Model::getRangeOfWeek() {
    QDate monday = QDate::currentDate().addDays(-QDate::currentDate().dayOfWeek() + 1) ;
    QDate sunday = monday.addDays(6);
    return {monday, sunday};
}

void Schedule_Model::getSchedule(short int id) {
    for (int i = 0; i < 7; i++) {
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
    if (!query.exec()) return;
    while (query.next()) {
        QDate date = query.value("workDate").toDate();
        Shift* newShift = new Shift(query.value("idEmployee").toInt(), date,
                                    query.value("startTime").toTime(), query.value("endTime").toTime());
        int dayInWeek = weekRange.first.daysTo(date); // monday: 0, sunday: 6
        if (dayInWeek >= 0 && dayInWeek < 7) {
            this->shiftList[dayInWeek].append(newShift);
            this->numberOfShift++;
        }
    }
}

// when a user create a shift, a temporary shift is created and add to the system's sort algorithm,
// only then the database will add approved shifts

// flow in control: select shift -> model check overlapping -> return preview -> submit -> system handle -> add shift to database
Shift* Schedule_Model::handleAddShiftSubmission(short int id, QDate date, QTime start, QTime end) {
    if (checkOverlapping(id, date, start, end)) {
        qDebug() << "Overlapped";
        return nullptr;
    }
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    if (!openData.transaction()) {
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
    Shift* newShift = new Shift(id, date, start, end);
    return newShift;
}

Schedule_Model::~Schedule_Model() {
    for (auto& dayShifts : shiftList) {
        qDeleteAll(dayShifts);
    }
}