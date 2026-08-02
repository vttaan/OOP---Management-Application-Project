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
    QString role;           // "Staff" | "Manager"
    QTime   startTime;
    QTime   endTime;
    short   status = 0;         // 0 = pending, 1 = approved, -1 = declined
};

// Count breakdown per shift block for the assign grid.
struct BlockCounts {
    int pending = 0;
    int accepted = 0;
    int declined = 0;
};

enum class EmployeeScheduleLayoutMode {
    PartTimeHourly,
    FullTimeMock
};

enum class FullTimeShiftStatus {
    Unregistered,
    Registered,
    StaffShortage
};

// Outer list index = day (Monday-Sunday), inner list index = shift (morning-evening).
using FullTimeScheduleGrid = QList<QList<FullTimeShiftStatus>>;
