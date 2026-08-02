#include "global.h"
#ifndef SHIFT_H
#define SHIFT_H

using namespace std;
class Shift
{
private:
    QDate date;
    short int EmployeeID;
    short int dayOfWeek;
    QTime startTime, endTime;
    QTime assignedStartTime, assignedEndTime;
    short int status;
    int shiftId;
public:
    Shift(short int id, QDate date, QTime start, QTime end);
    void setDayOfWeek();
    void setTime(QTime start, QTime end);
    void setStatus(short int status);
    QTime getTotalHourWork();
    QDate getDate() const;
    short int getEmployeeID() const;
    QTime getStartTime() const;
    QTime getEndTime() const;
    short int getStatus() const;


    void setAssignedTime(QTime start, QTime end);
    QTime getAssignedStartTime() const;
    QTime getAssignedEndTime() const;

    int getShiftId() const;
    void setShiftId(int id);

    void setID(short int id);
    ~Shift() = default;
};
#endif // SHIFT_H