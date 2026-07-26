#ifndef SCHEDULE_MODEL_H
#define SCHEDULE_MODEL_H

#include "global.h"
#include "core/Shift.h"
#include "utils/Database.h"

#include "core/ShiftBlock.h"
#include <QMap>
#include "core/Optimizer.h"
class Schedule_Model
{
private:
    QList<QList<Shift *>> shiftList{7}; // 7 days
    int numberOfShift;
    QList<User*> currentWeeklyUsers;
    QList<Shift*> draftShifts;

    QVector<Shift*> fetchPendingShifts(const QDate& weekStart, const QDate& weekEnd);
    // Lay tong so phut da lam cua tung nhan vien
    QMap<User*, int> fetchAllEmployeeInfos(const QDate& weekStart);
public:
    Schedule_Model();
    bool checkOverlapping(short int id, QDate date, QTime start, QTime end);
    Shift *getPreviewShift(short int id, QDate date, QTime start, QTime end);
    Shift *handleAddShiftSubmission(short int id, QDate date, QTime start, QTime end);
    void getSchedule(short int id, QDate monday);

    // Returns the in-memory weekly shift list (index 0=Mon, 6=Sun)
    const QList<QList<Shift *>>& getShiftList() const { return shiftList; }


    void getAcceptedSchedule(short int id, QDate monday);
    QMap<int, QMap<int, ShiftBlock*>> getManagerWeeklyGrid(QDate monday, int status = 1);
    QMap<int, QList<QString>> getWeeklySummaryStrings() const;

    QStringList generateSchedule();
    bool saveDraftShiftsToDatabase();
    void clearDrafts() {draftShifts.clear();}

    ~Schedule_Model();
};

#endif // SCHEDULE_MODEL_H
