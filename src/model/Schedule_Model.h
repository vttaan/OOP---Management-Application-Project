#ifndef SCHEDULE_MODEL_H
#define SCHEDULE_MODEL_H

#include "global.h"
#include "core/Shift.h"
#include "utils/Database.h"

#include "core/ShiftBlock.h"
#include <QMap>
class Schedule_Model
{
private:
    QList<QList<Shift *>> shiftList{7}; // 7 days
    // maybe add qlist temporary shift list
    int numberOfShift;
    QList<User*> currentWeeklyUsers;

public:
    Schedule_Model();
    bool checkOverlapping(short int id, QDate date, QTime start, QTime end);
    Shift *getPreviewShift(short int id, QDate date, QTime start, QTime end);
    Shift *handleAddShiftSubmission(short int id, QDate date, QTime start, QTime end);
    void getSchedule(short int id);
    pair<QDate, QDate> getRangeOfWeek();

    // Returns the in-memory weekly shift list (index 0=Mon, 6=Sun)
    const QList<QList<Shift *>>& getShiftList() const { return shiftList; }

    void getAcceptedSchedule(short int id, QDate monday);
    QMap<int, QMap<int, ShiftBlock*>> getManagerWeeklyGrid(QDate monday);
    QMap<int, QList<QString>> getWeeklySummaryStrings() const;
    ~Schedule_Model();
};

#endif // SCHEDULE_MODEL_H
