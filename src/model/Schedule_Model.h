#ifndef SCHEDULE_MODEL_H
#define SCHEDULE_MODEL_H

#include "global.h"
#include "core/Shift.h"
#include "utils/Database.h"

class Schedule_Model
{
private:
    QList<Shift> currentShifts;

public:
    Schedule_Model();
    void setShifts(const QList<Shift>& shifts);
    QList<Shift> getShifts() const;
    
    // Check if the given shift overlaps with any in currentShifts
    bool checkOverlapping(short int id, QDate date, QTime start, QTime end);
};

#endif // SCHEDULE_MODEL_H
