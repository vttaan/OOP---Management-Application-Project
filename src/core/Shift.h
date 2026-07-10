#include "global.h"
#ifndef SHIFT_H
#define SHIFT_H

using namespace std;
class Shift
{
private:
    QDate date;
    short int EmployeeID;
    short int dayOfWeek; // 1: Sunday --> 7: Saturday
    QTime startTime, endTime;
    short int status; // 0: pending, 1: approved, 2: working, 3: completed,...
public:
    Shift(short int id, QDate date, QTime start, QTime end);
    void setDayOfWeek();
    void setTime(QTime start, QTime end);
    QTime getTotalHourWork();
    QDate getDate() const;
    short int getEmployeeID() const;
    QTime getStartTime() const;
    QTime getEndTime() const;
    short int getStatus() const;
    
    void setID(short int id);
};
#endif // SHIFT_H
