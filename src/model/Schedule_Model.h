#ifndef SCHEDULE_MODEL_H
#define SCHEDULE_MODEL_H

#include "global.h"
#include "core/Shift.h"
#include "utils/Database.h"
#include "core/Optimizer.h"
class Schedule_Model
{
private:
    QList<QList<Shift *>> shiftList{7}; // 7 days
    int numberOfShift;
    QVector<ShiftRegistration> fetchPendingShifts(const QDate& weekStart, const QDate& weekEnd);
    // Lấy tổng số phút đã làm của từng nhân viên
    QVector<EmployeeInfo> fetchAllEmployeeInfos(const QDate& weekStart);
public:
    Schedule_Model();
    bool checkOverlapping(short int id, QDate date, QTime start, QTime end);
    Shift *getPreviewShift(short int id, QDate date, QTime start, QTime end);
    Shift *handleAddShiftSubmission(short int id, QDate date, QTime start, QTime end);
    void getSchedule(short int id);
    pair<QDate, QDate> getRangeOfWeek();

    // Returns the in-memory weekly shift list (index 0=Mon, 6=Sun)
    const QList<QList<Shift *>>& getShiftList() const { return shiftList; }
     OptimizerOutput generateSchedule();
    ~Schedule_Model();
};

#endif // SCHEDULE_MODEL_H
