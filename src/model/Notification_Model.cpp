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
    QString sql = "SELECT id, recipientEmployeeId, type, title, message, status, "
                  "relatedShiftId, relatedLeaveRequestId, createdAt, readAt "
                  "FROM NOTIFICATION WHERE recipientEmployeeId = :employee";
    if (filter == "Unread")
        sql += " AND status = 'Unread'";
    else if (filter == "Shift")
        sql += " AND type LIKE 'SHIFT_%'";
    else if (filter == "Leave")
        sql += " AND type LIKE 'LEAVE_%'";
    sql += " ORDER BY createdAt DESC, id DESC";

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
        info.relatedShiftId = query.value(6).toInt();
        info.relatedLeaveRequestId = query.value(7).toInt();
        info.createdAt = readDateTime(query.value(8));
        info.readAt = readDateTime(query.value(9));
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

bool Notification_Model::create(QSqlDatabase &database, int recipientEmployeeId,
                                const QString &type, const QString &title,
                                const QString &message, int relatedShiftId,
                                int relatedLeaveRequestId) {
    if (recipientEmployeeId <= 0 || type.isEmpty() || title.isEmpty())
        return false;
    QSqlQuery query(database);
    query.prepare("INSERT INTO NOTIFICATION "
                  "(recipientEmployeeId, type, title, message, status, relatedShiftId, "
                  "relatedLeaveRequestId, createdAt) "
                  "VALUES (:recipient, :type, :title, :message, 'Unread', :shift, :leave, :at)");
    query.bindValue(":recipient", recipientEmployeeId);
    query.bindValue(":type", type);
    query.bindValue(":title", title);
    query.bindValue(":message", message);
    query.bindValue(":shift", relatedShiftId > 0 ? QVariant(relatedShiftId) : QVariant());
    query.bindValue(":leave", relatedLeaveRequestId > 0 ? QVariant(relatedLeaveRequestId) : QVariant());
    query.bindValue(":at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    return query.exec();
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
