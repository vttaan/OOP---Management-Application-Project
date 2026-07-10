#include "global.h"
#include "Shift.h"

Shift::Shift(short int id, QDate date, QTime start, QTime end) {
    this->status = 0; //pending
    this->EmployeeID = id;
    this->date = date;
    this->startTime = start;
    this->endTime = end;
    setDayOfWeek();
}

void Shift::setDayOfWeek() {
    short int m = date.month();
    short int k = date.year() % 100; // last 2 digits of year
    short int j = date.year() / 100; // first 2 digits of year
    if (m < 3) m += 12; // jan: 13, feb: 14
    this->dayOfWeek = (date.day() + (13 / 5 * (m + 1)) + k + (k / 4) + (j / 4) + (2 * j)) % 7;
    if (this->dayOfWeek == 0) this->dayOfWeek = 7;
}

QTime Shift::getTotalHourWork() {
    int diff = startTime.msecsTo(endTime);
    return QTime::fromMSecsSinceStartOfDay(diff);
}

void Shift::setID(short int id) { this->EmployeeID = id; }
void Shift::setTime(QTime start, QTime end) {
    // if (not overlap) {
    this->startTime = start;
    this->endTime = end;
}
