#pragma once

#include "global.h"
#include <QDateTime>

struct NotificationInfo {
    int id = 0;
    int recipientEmployeeId = 0;
    QString type;
    QString title;
    QString message;
    QString status = "Unread";
    int relatedShiftId = 0;
    int relatedLeaveRequestId = 0;
    QString relatedLeaveRequestStatus;
    QDateTime createdAt;
    QDateTime readAt;
};

struct LeaveRequestInfo {
    int id = 0;
    int employeeId = 0;
    QString employeeName;
    QDate leaveDate;
    int relatedShiftId = 0;
    QString reason;
    QString status = "Pending";
    QDateTime requestedAt;
    QDateTime decidedAt;
    int decidedBy = 0;
    QString decisionReason;
};

struct LeaveShiftOption {
    int shiftId = 0;
    QDate date;
    QTime startTime;
    QTime endTime;
    int status = 0;
};
