#ifndef SHIFT_H
#define SHIFT_H
#include <QString>
#include <QDate>
#include <QTime>
#include <cmath>
using namespace std;
class Shift {
private:
    QDate date; // yyyy-mm-dd
    short int EmployeeID;
    short int dayOfWeek; // 1: Sunday --> 7: Saturday
    QTime startTime, endTime;
    short int status; // 0: pending, 1: approved, -1: declined,...
public:
    Shift(short int id, QDate date, QTime start, QTime end);
    void setDayOfWeek();
    void setTime(QTime start, QTime end);
    void setStatus(short int status);
    QTime getTotalHourWork();
    void setID(short int id);
    ~Shift() = default;
};
#endif // SHIFT_H
