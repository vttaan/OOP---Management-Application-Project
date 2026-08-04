#pragma once
#include "global.h"

// ─────────────────────────────────────────────────────────────────────────────
// Shared Data Transfer Objects for the Schedule feature.
// Included by both Schedule_Model (model) and Schedule_View (view)
// via their respective headers. Neither layer owns these types.
// ─────────────────────────────────────────────────────────────────────────────

// Carries all information for a single pending/approved/declined shift request.
// Used by the manager's Xep Lich Lam popup dialog.
struct PendingShiftInfo {
    int     shiftId = 0;        // DB rowid — used to approve / decline
    int     employeeId = 0;
    QString employeeName;
    QString role;           // Manager/Admin or a concrete staff role from PROFILES
    QDate   date;
    QTime   startTime;
    QTime   endTime;
    short   status = 0;         // 0 = pending, 1 = approved, -1 = declined
};

// Count breakdown per shift block for the assign grid.
struct BlockCounts {
    int pending = 0;
    int accepted = 0;
    int declined = 0;
    int required = 0;
    int cancelled = 0;
};

enum class ManagerScheduleChangeType {
    Approve,
    Decline,
    Add,
    Cancel
};

// A manager edit is kept in memory until the weekly confirmation action.
// This keeps the grid reversible and lets the model commit all edits atomically.
struct ManagerScheduleChange {
    ManagerScheduleChangeType type = ManagerScheduleChangeType::Add;
    int shiftId = 0;
    int employeeId = 0;
    QString employeeName;
    QString role;
    QDate date;
    QTime startTime;
    QTime endTime;
    QString reason;
};

// Dry-run result from the optimizer. Proposed changes stay in memory until the
// manager accepts the preview and later publishes the weekly draft.
struct AutoSchedulePreview {
    QList<ManagerScheduleChange> changes;
    QStringList warnings;
    int approvedCount = 0;
    int declinedCount = 0;
};

struct EligibleEmployeeInfo {
    int employeeId = 0;
    QString employeeName;
    QString role;
    bool isFixedSalary = false;
    bool eligible = false;
    QString reason;
};

// Controller-produced time range used to synchronize an employee's editable
// pending registrations without exposing table row coordinates to the model.
struct StaffShiftRegistration {
    QDate date;
    QTime startTime;
    QTime endTime;
};

enum class EmployeeScheduleLayoutMode {
    PartTimeHourly,
    FullTimeSchedule
};

inline EmployeeScheduleLayoutMode scheduleLayoutModeForPayType(
    bool isFixedSalary)
{
    return isFixedSalary
        ? EmployeeScheduleLayoutMode::FullTimeSchedule
        : EmployeeScheduleLayoutMode::PartTimeHourly;
}

enum class FullTimeShiftStatus {
    Available,
    Pending,
    Approved,
    Declined,
    StaffShortage
};

// Outer list index = day (Monday-Sunday), inner list index = shift (morning-evening).
using FullTimeScheduleGrid = QList<QList<FullTimeShiftStatus>>;
