#include "global.h"
#include "model/Notification_Model.h"

namespace {
QDateTime readDateTime(const QVariant &value) {
    QDateTime result = value.toDateTime();
    if (!result.isValid())
        result = QDateTime::fromString(value.toString(), Qt::ISODate);
    return result;
}
}

QList<NotificationInfo> Notification_Model::getNotifications(
    int employeeId, const QString &filter) const {
    QList<NotificationInfo> result;
    if (employeeId <= 0)
        return result;

    QSqlQuery query(Database::getInstance()->getDbConnect());
    QString sql = "SELECT N.id, N.recipientEmployeeId, N.type, N.title, N.message, N.status, "
                  "N.priority, N.dedupeKey, N.relatedShiftId, N.relatedLeaveRequestId, L.status, "
                  "N.createdAt, N.readAt "
                  "FROM NOTIFICATION N LEFT JOIN LEAVE_REQUEST L "
                  "ON L.id = N.relatedLeaveRequestId "
                  "WHERE N.recipientEmployeeId = :employee";
    if (filter == "Unread")
        sql += " AND N.status = 'Unread'";
    else if (filter == "Shift")
        sql += " AND (N.type LIKE 'SHIFT_%' OR N.type = 'STAFFING_SHORTAGE')";
    else if (filter == "Leave")
        sql += " AND N.type LIKE 'LEAVE_%'";
    sql += " ORDER BY N.priority DESC, "
           "CASE WHEN N.status = 'Unread' THEN 0 ELSE 1 END, "
           "N.createdAt DESC, N.id DESC";

    query.prepare(sql);
    query.bindValue(":employee", employeeId);
    if (!query.exec())
        return result;

    while (query.next()) {
        NotificationInfo info;
        info.id = query.value(0).toInt();
        info.recipientEmployeeId = query.value(1).toInt();
        info.type = query.value(2).toString();
        info.title = query.value(3).toString();
        info.message = query.value(4).toString();
        info.status = query.value(5).toString();
        info.priority = query.value(6).toInt();
        info.dedupeKey = query.value(7).toString();
        info.relatedShiftId = query.value(8).toInt();
        info.relatedLeaveRequestId = query.value(9).toInt();
        info.relatedLeaveRequestStatus = query.value(10).toString();
        info.createdAt = readDateTime(query.value(11));
        info.readAt = readDateTime(query.value(12));
        result.append(info);
    }
    return result;
}

int Notification_Model::getUnreadCount(int employeeId) const {
    if (employeeId <= 0)
        return 0;
    QSqlQuery query(Database::getInstance()->getDbConnect());
    query.prepare("SELECT COUNT(*) FROM NOTIFICATION "
                  "WHERE recipientEmployeeId = :employee AND status = 'Unread'");
    query.bindValue(":employee", employeeId);
    return query.exec() && query.next() ? query.value(0).toInt() : 0;
}

bool Notification_Model::markAsRead(int notificationId, int employeeId) const {
    if (notificationId <= 0 || employeeId <= 0)
        return false;
    QSqlQuery query(Database::getInstance()->getDbConnect());
    query.prepare("UPDATE NOTIFICATION SET status = 'Read', readAt = :at "
                  "WHERE id = :id AND recipientEmployeeId = :employee "
                  "AND status = 'Unread'");
    query.bindValue(":at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    query.bindValue(":id", notificationId);
    query.bindValue(":employee", employeeId);
    return query.exec();
}

bool Notification_Model::markLeaveRequestReviewed(int notificationId,
                                                   int employeeId,
                                                   bool approved) const {
    if (notificationId <= 0 || employeeId <= 0)
        return false;

    QSqlQuery query(Database::getInstance()->getDbConnect());
    query.prepare("UPDATE NOTIFICATION SET type = :type, title = :title, "
                  "status = 'Read', readAt = :at "
                  "WHERE id = :id AND recipientEmployeeId = :employee "
                  "AND type = 'LEAVE_SUBMITTED'");
    query.bindValue(":type", approved ? "LEAVE_APPROVED" : "LEAVE_DECLINED");
    query.bindValue(":title", approved
                    ? QString::fromUtf8("Yêu cầu nghỉ phép đã được duyệt")
                    : QString::fromUtf8("Yêu cầu nghỉ phép đã bị từ chối"));
    query.bindValue(":at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    query.bindValue(":id", notificationId);
    query.bindValue(":employee", employeeId);
    return query.exec();
}

bool Notification_Model::markLeaveRequestReviewedByRequest(int leaveRequestId,
                                                            bool approved) const {
    if (leaveRequestId <= 0)
        return false;

    QSqlQuery query(Database::getInstance()->getDbConnect());
    query.prepare("UPDATE NOTIFICATION SET type = :type, title = :title, "
                  "status = 'Read', readAt = :at "
                  "WHERE relatedLeaveRequestId = :leave "
                  "AND type = 'LEAVE_SUBMITTED'");
    query.bindValue(":type", approved ? "LEAVE_APPROVED" : "LEAVE_DECLINED");
    query.bindValue(":title", approved
                    ? QString::fromUtf8("Yêu cầu nghỉ phép đã được duyệt")
                    : QString::fromUtf8("Yêu cầu nghỉ phép đã bị từ chối"));
    query.bindValue(":at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    query.bindValue(":leave", leaveRequestId);
    return query.exec();
}

bool Notification_Model::markAllAsRead(int employeeId) const {
    if (employeeId <= 0)
        return false;
    QSqlQuery query(Database::getInstance()->getDbConnect());
    query.prepare("UPDATE NOTIFICATION SET status = 'Read', readAt = :at "
                  "WHERE recipientEmployeeId = :employee AND status = 'Unread'");
    query.bindValue(":at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    query.bindValue(":employee", employeeId);
    return query.exec();
}

bool Notification_Model::deleteAllRead(int employeeId) const {
    if (employeeId <= 0)
        return false;
    QSqlQuery query(Database::getInstance()->getDbConnect());
    query.prepare("DELETE FROM NOTIFICATION "
                  "WHERE recipientEmployeeId = :employee AND status = 'Read'");
    query.bindValue(":employee", employeeId);
    return query.exec();
}

bool Notification_Model::create(QSqlDatabase &database, int recipientEmployeeId,
                                const QString &type, const QString &title,
                                const QString &message, int relatedShiftId,
                                int relatedLeaveRequestId, int priority,
                                const QString &dedupeKey) {
    if (recipientEmployeeId <= 0 || type.isEmpty() || title.isEmpty())
        return false;
    QSqlQuery query(database);
    query.prepare("INSERT INTO NOTIFICATION "
                  "(recipientEmployeeId, type, title, message, status, priority, dedupeKey, "
                  "relatedShiftId, relatedLeaveRequestId, createdAt) "
                  "VALUES (:recipient, :type, :title, :message, 'Unread', :priority, :dedupe, "
                  ":shift, :leave, :at)");
    query.bindValue(":recipient", recipientEmployeeId);
    query.bindValue(":type", type);
    query.bindValue(":title", title);
    query.bindValue(":message", message);
    query.bindValue(":priority", priority);
    query.bindValue(":dedupe", dedupeKey.isEmpty() ? QVariant() : QVariant(dedupeKey));
    query.bindValue(":shift", relatedShiftId > 0 ? QVariant(relatedShiftId) : QVariant());
    query.bindValue(":leave", relatedLeaveRequestId > 0 ? QVariant(relatedLeaveRequestId) : QVariant());
    query.bindValue(":at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    return query.exec();
}

bool Notification_Model::createIfAbsent(QSqlDatabase &database, int recipientEmployeeId,
                                        const QString &type, const QString &title,
                                        const QString &message, int priority,
                                        const QString &dedupeKey, int relatedShiftId,
                                        int relatedLeaveRequestId) {
    if (dedupeKey.isEmpty())
        return create(database, recipientEmployeeId, type, title, message,
                      relatedShiftId, relatedLeaveRequestId, priority);

    QSqlQuery existing(database);
    existing.prepare("SELECT 1 FROM NOTIFICATION WHERE recipientEmployeeId = :recipient "
                     "AND dedupeKey = :dedupe LIMIT 1");
    existing.bindValue(":recipient", recipientEmployeeId);
    existing.bindValue(":dedupe", dedupeKey);
    if (!existing.exec())
        return false;
    if (existing.next())
        return true;

    return create(database, recipientEmployeeId, type, title, message,
                  relatedShiftId, relatedLeaveRequestId, priority, dedupeKey);
}

QList<int> Notification_Model::getManagerRecipientIds(QSqlDatabase &database) {
    QList<int> result;
    QSqlQuery query(database);
    if (!query.exec("SELECT idEmployee FROM PROFILES "
                    "WHERE role IN ('Manager', 'Admin')"))
        return result;
    while (query.next())
        result.append(query.value(0).toInt());
    return result;
}
