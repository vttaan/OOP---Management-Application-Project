#include "global.h"
#include "Shift.h"

Shift::Shift(short int id, QDate date, QTime start, QTime end)
    : status(0), EmployeeID(id), date(date), startTime(start), endTime(end)
    , assignedStartTime(start), assignedEndTime(end) // mac dinh = gio dang ky, Optimizer se ghi de neu can
{
    setDayOfWeek();
}

void Shift::setDayOfWeek()
{
    short int m = date.month();
    short int k = date.year() % 100; // last 2 digits of year
    short int j = date.year() / 100; // first 2 digits of year
    if (m < 3)
        m += 12; // jan: 13, feb: 14
    this->dayOfWeek = (date.day() + ((13 * (m + 1)) / 5) + k + (k / 4) + (j / 4) + (2 * j)) % 7;
    if (this->dayOfWeek == 0)
        this->dayOfWeek = 7;
}

QTime Shift::getTotalHourWork()
{
    int diff = startTime.msecsTo(endTime);
    return QTime::fromMSecsSinceStartOfDay(diff);
}

void Shift::setID(short int id) { this->EmployeeID = id; }
int Shift::getShiftId() const { return shiftId; }
void Shift::setShiftId(int id) { this->shiftId = id; }
void Shift::setTime(QTime start, QTime end)
{
    // if (not overlap) {
    this->startTime = start;
    this->endTime = end;
}

void Shift::setStatus(short int status) { this->status = status; }

QDate Shift::getDate() const { return date; }
short int Shift::getEmployeeID() const { return EmployeeID; }
QTime Shift::getStartTime() const { return startTime; }
QTime Shift::getEndTime() const { return endTime; }
short int Shift::getStatus() const { return status; }

void Shift::setAssignedTime(QTime start, QTime end) {
    this->assignedStartTime = start;
    this->assignedEndTime = end;
}
QTime Shift::getAssignedStartTime() const { return assignedStartTime; }
QTime Shift::getAssignedEndTime() const { return assignedEndTime; }