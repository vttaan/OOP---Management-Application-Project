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

// Count breakdown for one operational role inside a canonical shift block.
struct RoleBlockCounts {
    int pending = 0;
    int accepted = 0;
    int declined = 0;
    int required = 0;
    int cancelled = 0;

    int missingSlots() const
    {
        return qMax(0, required - accepted);
    }
};

// Count breakdown per shift block for the assign grid. Aggregate fields retain
// the existing UI contract; byRole is authoritative for staffing sufficiency.
struct BlockCounts {
    int pending = 0;
    int accepted = 0;
    int declined = 0;
    int required = 0;
    int cancelled = 0;
    QMap<QString, RoleBlockCounts> byRole;

    void recalculateTotals()
    {
        pending = 0;
        accepted = 0;
        declined = 0;
        required = 0;
        cancelled = 0;
        for (const RoleBlockCounts &counts : byRole)
        {
            pending += counts.pending;
            accepted += counts.accepted;
            declined += counts.declined;
            required += counts.required;
            cancelled += counts.cancelled;
        }
    }

    int missingSlots() const
    {
        int missing = 0;
        for (const RoleBlockCounts &counts : byRole)
            missing += counts.missingSlots();
        return missing;
    }

    int coveredRequiredSlots() const
    {
        return qMax(0, required - missingSlots());
    }

    bool hasShortage() const
    {
        return missingSlots() > 0;
    }

    QMap<QString, int> missingByRole() const
    {
        QMap<QString, int> missing;
        for (auto it = byRole.constBegin(); it != byRole.constEnd(); ++it)
        {
            const int deficit = it.value().missingSlots();
            if (deficit > 0)
                missing.insert(it.key(), deficit);
        }
        return missing;
    }

    QString deficitSignature() const
    {
        QStringList parts;
        const QMap<QString, int> missing = missingByRole();
        for (auto it = missing.constBegin(); it != missing.constEnd(); ++it)
            parts.append(QString("%1:%2").arg(it.key()).arg(it.value()));
        return parts.join(",");
    }

    void adjustStatus(const QString &role, int status, int delta)
    {
        if (role.trimmed().isEmpty() || delta == 0)
            return;
        RoleBlockCounts &counts = byRole[role];
        int *target = nullptr;
        if (status == 0)
            target = &counts.pending;
        else if (status == 1)
            target = &counts.accepted;
        else if (status == -1)
            target = &counts.declined;
        else if (status == -2)
            target = &counts.cancelled;
        if (!target)
            return;
        *target = qMax(0, *target + delta);
        recalculateTotals();
    }
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

// Manager-reviewed employee selection produced by the multi-select chooser.
// The view keeps these selections scoped to one shift block; the controller
// converts them into ManagerScheduleChange entries for the weekly draft.
struct ManagerEmployeeSelection {
    int employeeId = 0;
    QString employeeName;
    QString role;
    bool isFixedSalary = false;
    QTime startTime;
    QTime endTime;
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
    StaffShortage,
    StaffSufficient
};

// Outer list index = day (Monday-Sunday), inner list index = shift (morning-evening).
using FullTimeScheduleGrid = QList<QList<FullTimeShiftStatus>>;
