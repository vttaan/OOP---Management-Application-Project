#ifndef SCHEDULE_MODEL_H
#define SCHEDULE_MODEL_H

#include "global.h"
#include "core/Shift.h"
#include "utils/Database.h"

class Schedule_Model
{
private:
    QList<QList<Shift *>> shiftList{7}; // 7 days
    // maybe add qlist temporary shift list
    int numberOfShift;

public:
    Schedule_Model();
    bool checkOverlapping(short int id, QDate date, QTime start, QTime end);
    Shift *getPreviewShift(short int id, QDate date, QTime start, QTime end);
    Shift *handleAddShiftSubmission(short int id, QDate date, QTime start, QTime end);
    void getSchedule(short int id);
    pair<QDate, QDate> getRangeOfWeek();
    ~Schedule_Model();
signals:
    void handleAddShiftSubmission(); // put this to schedule controller
    void updateStatusSignal();
};

#endif // SCHEDULE_MODEL_H
