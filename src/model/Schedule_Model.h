#ifndef SCHEDULE_MODEL_H
#define SCHEDULE_MODEL_H

#include "global.h"
#include "core/Shift.h"
#include "utils/Database.h"
#include "utils/ScheduleDTOs.h"

#include "core/ShiftBlock.h"
#include <QMap>
#include "core/Optimizer.h"

class Schedule_Model
{
private:
    QList<QList<Shift *>> shiftList{7}; // 7 days
    int numberOfShift;
    QList<User *> currentWeeklyUsers;
    QList<Shift *> draftShifts;

    QVector<Shift *> fetchPendingShifts(const QDate &weekStart, const QDate &weekEnd);
    // Lấy tổng số phút đã làm của từng nhân viên
    QMap<User *, int> fetchAllEmployeeInfos(const QDate &weekStart);

public:
    Schedule_Model();
    bool checkOverlapping(short int id, QDate date, QTime start, QTime end);
    Shift *getPreviewShift(short int id, QDate date, QTime start, QTime end);
    Shift *handleAddShiftSubmission(short int id, QDate date, QTime start, QTime end);
    void getSchedule(short int id, QDate monday);

    // Returns the in-memory weekly shift list (index 0=Mon, 6=Sun)
    const QList<QList<Shift *>> &getShiftList() const { return shiftList; }

    void getAcceptedSchedule(short int id, QDate monday);
    void getPendingSchedule(short int id, QDate monday);
    QMap<int, QMap<int, ShiftBlock *>> getManagerWeeklyGrid(QDate monday, int status = 1);
    QMap<int, QList<QString>> getWeeklySummaryStrings() const;

    // Xếp Lịch Làm — assignment grid: fetches all statuses (pending + accepted + declined)
    // grid: col (0-6 Mon-Sun) -> row (0-2 shift) -> { pendingCount, acceptedCount, declinedCount }
    QMap<int, QMap<int, BlockCounts>> getAssignBlockCounts(QDate monday);

    // Returns all shift requests that touch a given shift block (col=day 0-6, row=shift 0-2)
    QList<PendingShiftInfo> getShiftsForBlock(QDate monday, int col, int row);

    // Employees that can be considered for a manager override. The result also
    // contains ineligible employees with a human-readable reason for the chooser.
    QList<EligibleEmployeeInfo> getEligibleEmployees(QDate date,
                                                     QTime startTime,
                                                     QTime endTime);

    // Applies a manager's reviewed weekly draft in one transaction. Approved
    // removals are retained as status -2 (cancelled), never hard-deleted.
    bool applyManagerScheduleChanges(const QList<ManagerScheduleChange> &changes,
                                     QStringList *errors = nullptr);
    QStringList validateManagerScheduleChanges(
        const QList<ManagerScheduleChange> &changes) const;
    AutoSchedulePreview previewGeneratedSchedule(QDate weekStart);

    // Raw shifts for 15x7 rendering
    QMap<int, QList<Shift*>> getRawStaffShifts(short int id, QDate monday, int status);

    // Database-backed 3x7 schedule for fixed-salary employees.
    FullTimeScheduleGrid getFullTimeScheduleGrid(short int employeeId,
                                                 QDate weekStart);

    // Approve or decline a shift by its DB rowid; returns true on success
    bool approveShift(int shiftId);
    bool declineShift(int shiftId);

signals:

    QStringList generateSchedule();
    bool saveDraftShiftsToDatabase();
    bool replacePendingShiftsForWeek(
        short int employeeId,
        QDate weekStart,
        const QList<StaffShiftRegistration> &registrations);
    void clearDrafts() { draftShifts.clear(); }

    ~Schedule_Model();
};

#endif // SCHEDULE_MODEL_H
