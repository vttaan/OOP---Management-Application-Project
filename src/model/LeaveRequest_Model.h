#pragma once

#include "global.h"
#include "utils/NotificationDTOs.h"

class LeaveRequest_Model {
public:
    QList<LeaveShiftOption> getActiveShiftsForWeek(int employeeId,
                                                    QDate weekStart) const;
    bool submitLeaveRequest(int employeeId, int shiftId, const QString &reason,
                            QString *error = nullptr) const;
    QList<LeaveRequestInfo> getLeaveRequestsForEmployee(int employeeId) const;
    bool decideLeaveRequest(int requestId, int managerId, bool approved,
                            const QString &decisionReason,
                            QString *error = nullptr) const;
};
