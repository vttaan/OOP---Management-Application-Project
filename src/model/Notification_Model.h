#pragma once

#include "global.h"
#include "utils/NotificationDTOs.h"

class Notification_Model {
public:
    QList<NotificationInfo> getNotifications(int employeeId,
                                             const QString &filter = {}) const;
    int getUnreadCount(int employeeId) const;
    bool markAsRead(int notificationId, int employeeId) const;
    bool markLeaveRequestReviewed(int notificationId, int employeeId,
                                  bool approved) const;
    bool markLeaveRequestReviewedByRequest(int leaveRequestId, bool approved) const;
    bool markAllAsRead(int employeeId) const;

    static bool create(QSqlDatabase &database, int recipientEmployeeId,
                       const QString &type, const QString &title,
                       const QString &message, int relatedShiftId = 0,
                       int relatedLeaveRequestId = 0);
    static QList<int> getManagerRecipientIds(QSqlDatabase &database);
};
