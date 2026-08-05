#include "global.h"
#include "model/LeaveRequest_Model.h"
#include "model/Notification_Model.h"
#include "utils/Config.h"

namespace {
QString displayDate(const QDate &date)
{
    return date.toString("dd/MM/yyyy");
}

QDateTime readDateTime(const QVariant &value)
{
    QDateTime result = value.toDateTime();
    if (!result.isValid())
        result = QDateTime::fromString(value.toString(), Qt::ISODate);
    return result;
}

QDate databaseDate(const QVariant &value)
{
    const QDate date = value.toDate();
    return date.isValid() ? date : QDate::fromString(value.toString(), Qt::ISODate);
}

QTime databaseTime(const QVariant &value)
{
    const QTime time = value.toTime();
    if (time.isValid())
        return time;
    const QString text = value.toString();
    for (const QString &format : {QString("HH:mm:ss"), QString("HH:mm"),
                                  QString("H:mm:ss"), QString("H:mm")}) {
        const QTime parsed = QTime::fromString(text, format);
        if (parsed.isValid())
            return parsed;
    }
    return {};
}
}

QList<LeaveShiftOption> LeaveRequest_Model::getActiveShiftsForWeek(
    int employeeId, QDate weekStart) const
{
    QList<LeaveShiftOption> result;
    if (employeeId <= 0 || !weekStart.isValid())
        return result;

    QSqlQuery query(Database::getInstance()->getDbConnect());
    query.prepare("SELECT rowid, workDate, startTime, endTime, status FROM SHIFT "
                  "WHERE idEmployee = :employee AND status IN (0, 1) "
                  "AND workDate BETWEEN :start AND :end "
                  "ORDER BY workDate, startTime");
    query.bindValue(":employee", employeeId);
    query.bindValue(":start", weekStart);
    query.bindValue(":end", weekStart.addDays(6));
    if (!query.exec())
        return result;

    while (query.next()) {
        LeaveShiftOption option;
        option.shiftId = query.value(0).toInt();
        option.date = databaseDate(query.value(1));
        option.startTime = databaseTime(query.value(2));
        option.endTime = databaseTime(query.value(3));
        option.status = query.value(4).toInt();
        if (option.shiftId > 0 && option.date.isValid() &&
            option.startTime.isValid() && option.endTime.isValid())
            result.append(option);
    }
    return result;
}

bool LeaveRequest_Model::submitLeaveRequest(int employeeId, int shiftId,
                                            const QString &reason,
                                            QString *error) const
{
    if (employeeId <= 0 || shiftId <= 0 || reason.trimmed().isEmpty()) {
        if (error) *error = QString::fromUtf8("Vui lòng chọn ca làm và nhập lý do.");
        return false;
    }

    QSqlDatabase database = Database::getInstance()->getDbConnect();
    if (!database.transaction()) {
        if (error) *error = database.lastError().text();
        return false;
    }
    auto fail = [&](const QString &message) {
        database.rollback();
        if (error) *error = message;
        return false;
    };

    QSqlQuery shift(database);
    shift.prepare("SELECT workDate FROM SHIFT WHERE rowid = :shift "
                  "AND idEmployee = :employee AND status IN (0, 1)");
    shift.bindValue(":shift", shiftId);
    shift.bindValue(":employee", employeeId);
    if (!shift.exec())
        return fail(shift.lastError().text());
    if (!shift.next())
        return fail(QString::fromUtf8("Ca làm đã chọn không còn khả dụng."));
    const QDate leaveDate = databaseDate(shift.value(0));
    if (!leaveDate.isValid() || leaveDate < QDate::currentDate())
        return fail(QString::fromUtf8("Không thể tạo yêu cầu nghỉ cho ca đã qua."));

    QSqlQuery duplicate(database);
    duplicate.prepare("SELECT 1 FROM LEAVE_REQUEST WHERE idEmployee = :employee "
                      "AND leaveDate = :date AND status IN ('Pending', 'Approved')");
    duplicate.bindValue(":employee", employeeId);
    duplicate.bindValue(":date", leaveDate);
    if (!duplicate.exec())
        return fail(duplicate.lastError().text());
    if (duplicate.next())
        return fail(QString::fromUtf8("Bạn đã có yêu cầu nghỉ đang chờ hoặc đã được duyệt cho ngày này."));

    QSqlQuery profile(database);
    profile.prepare("SELECT isFixed FROM PROFILES WHERE idEmployee = :employee");
    profile.bindValue(":employee", employeeId);
    if (!profile.exec())
        return fail(profile.lastError().text());
    if (!profile.next())
        return fail(QString::fromUtf8("Không tìm thấy thông tin nhân viên."));
    if (profile.value(0).toBool()) {
        const int leaveLimit = Config::getMaximumLeavePerMonth_FT();
        QSqlQuery count(database);
        count.prepare("SELECT COUNT(*) FROM LEAVE_REQUEST WHERE idEmployee = :employee "
                      "AND status IN ('Pending', 'Approved') "
                      "AND strftime('%Y-%m', leaveDate) = :month");
        count.bindValue(":employee", employeeId);
        count.bindValue(":month", leaveDate.toString("yyyy-MM"));
        if (!count.exec() || !count.next())
            return fail(count.lastError().text());
        if (count.value(0).toInt() >= leaveLimit)
            return fail(QString::fromUtf8("Bạn đã đạt giới hạn nghỉ phép trong tháng này."));
    }

    QSqlQuery insert(database);
    insert.prepare("INSERT INTO LEAVE_REQUEST "
                   "(idEmployee, leaveDate, relatedShiftId, reason, status, requestedAt) "
                   "VALUES (:employee, :date, :shift, :reason, 'Pending', :at)");
    insert.bindValue(":employee", employeeId);
    insert.bindValue(":date", leaveDate);
    insert.bindValue(":shift", shiftId);
    insert.bindValue(":reason", reason.trimmed());
    insert.bindValue(":at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    if (!insert.exec())
        return fail(insert.lastError().text());
    const int requestId = insert.lastInsertId().toInt();

    for (int managerId : Notification_Model::getManagerRecipientIds(database)) {
        if (!Notification_Model::create(
                database, managerId, "LEAVE_SUBMITTED",
                QString::fromUtf8("Yêu cầu nghỉ phép mới"),
                QString::fromUtf8("Nhân viên #%1 xin nghỉ ngày %2.")
                    .arg(employeeId).arg(displayDate(leaveDate)),
                shiftId, requestId))
            return fail(QString::fromUtf8("Không thể tạo thông báo cho quản lý."));
    }

    if (!database.commit()) {
        if (error) *error = database.lastError().text();
        return false;
    }
    return true;
}

QList<LeaveRequestInfo> LeaveRequest_Model::getLeaveRequestsForEmployee(int employeeId) const
{
    QList<LeaveRequestInfo> result;
    if (employeeId <= 0)
        return result;
    QSqlQuery query(Database::getInstance()->getDbConnect());
    query.prepare("SELECT L.id, L.idEmployee, P.name, L.leaveDate, L.relatedShiftId, "
                  "L.reason, L.status, L.requestedAt, L.decidedAt, L.decidedBy, "
                  "L.decisionReason FROM LEAVE_REQUEST L "
                  "JOIN PROFILES P ON P.idEmployee = L.idEmployee "
                  "WHERE L.idEmployee = :employee ORDER BY L.requestedAt DESC");
    query.bindValue(":employee", employeeId);
    if (!query.exec())
        return result;
    while (query.next()) {
        LeaveRequestInfo info;
        info.id = query.value(0).toInt();
        info.employeeId = query.value(1).toInt();
        info.employeeName = query.value(2).toString();
        info.leaveDate = databaseDate(query.value(3));
        info.relatedShiftId = query.value(4).toInt();
        info.reason = query.value(5).toString();
        info.status = query.value(6).toString();
        info.requestedAt = readDateTime(query.value(7));
        info.decidedAt = readDateTime(query.value(8));
        info.decidedBy = query.value(9).toInt();
        info.decisionReason = query.value(10).toString();
        result.append(info);
    }
    return result;
}

QList<LeaveRequestInfo> LeaveRequest_Model::getPendingLeaveRequests() const
{
    QList<LeaveRequestInfo> result;
    QSqlQuery query(Database::getInstance()->getDbConnect());
    if (!query.exec(
            "SELECT L.id, L.idEmployee, P.name, L.leaveDate, L.relatedShiftId, "
            "L.reason, L.status, L.requestedAt, L.decidedAt, L.decidedBy, "
            "L.decisionReason FROM LEAVE_REQUEST L "
            "JOIN PROFILES P ON P.idEmployee = L.idEmployee "
            "WHERE L.status = 'Pending' ORDER BY L.requestedAt ASC, L.id ASC"))
        return result;

    while (query.next()) {
        LeaveRequestInfo info;
        info.id = query.value(0).toInt();
        info.employeeId = query.value(1).toInt();
        info.employeeName = query.value(2).toString();
        info.leaveDate = databaseDate(query.value(3));
        info.relatedShiftId = query.value(4).toInt();
        info.reason = query.value(5).toString();
        info.status = query.value(6).toString();
        info.requestedAt = readDateTime(query.value(7));
        info.decidedAt = readDateTime(query.value(8));
        info.decidedBy = query.value(9).toInt();
        info.decisionReason = query.value(10).toString();
        result.append(info);
    }
    return result;
}

bool LeaveRequest_Model::decideLeaveRequest(int requestId, int managerId, bool approved,
                                            const QString &decisionReason,
                                            QString *error) const
{
    if (requestId <= 0 || managerId <= 0) {
        if (error) *error = QString::fromUtf8("Yêu cầu hoặc quản lý không hợp lệ.");
        return false;
    }
    QSqlDatabase database = Database::getInstance()->getDbConnect();
    if (!database.transaction()) {
        if (error) *error = database.lastError().text();
        return false;
    }
    auto fail = [&](const QString &message) {
        database.rollback();
        if (error) *error = message;
        return false;
    };

    QSqlQuery managerProfile(database);
    managerProfile.prepare("SELECT role FROM PROFILES WHERE idEmployee = :manager");
    managerProfile.bindValue(":manager", managerId);
    if (!managerProfile.exec())
        return fail(managerProfile.lastError().text());
    if (!managerProfile.next() ||
        (managerProfile.value(0).toString() != "Manager" &&
         managerProfile.value(0).toString() != "Admin"))
        return fail(QString::fromUtf8("Chỉ quản lý hoặc quản trị viên mới có thể duyệt yêu cầu."));

    QSqlQuery requestQuery(database);
    requestQuery.prepare("SELECT idEmployee, leaveDate FROM LEAVE_REQUEST "
                         "WHERE id = :id AND status = 'Pending'");
    requestQuery.bindValue(":id", requestId);
    if (!requestQuery.exec())
        return fail(requestQuery.lastError().text());
    if (!requestQuery.next())
        return fail(QString::fromUtf8("Yêu cầu này không còn ở trạng thái chờ duyệt."));
    const int employeeId = requestQuery.value(0).toInt();
    const QDate leaveDate = databaseDate(requestQuery.value(1));

    const QString newStatus = approved ? "Approved" : "Declined";
    QSqlQuery updateRequest(database);
    updateRequest.prepare("UPDATE LEAVE_REQUEST SET status = :status, decidedAt = :at, "
                          "decidedBy = :manager, decisionReason = :reason "
                          "WHERE id = :id AND status = 'Pending'");
    updateRequest.bindValue(":status", newStatus);
    updateRequest.bindValue(":at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    updateRequest.bindValue(":manager", managerId);
    updateRequest.bindValue(":reason", decisionReason.trimmed());
    updateRequest.bindValue(":id", requestId);
    if (!updateRequest.exec() || updateRequest.numRowsAffected() != 1)
        return fail(updateRequest.lastError().text());

    if (approved) {
        QSqlQuery shifts(database);
        shifts.prepare("SELECT rowid FROM SHIFT WHERE idEmployee = :employee "
                       "AND workDate = :date AND status IN (0, 1)");
        shifts.bindValue(":employee", employeeId);
        shifts.bindValue(":date", leaveDate);
        if (!shifts.exec())
            return fail(shifts.lastError().text());
        QList<int> shiftIds;
        while (shifts.next()) shiftIds.append(shifts.value(0).toInt());

        QSqlQuery cancel(database);
        cancel.prepare("UPDATE SHIFT SET status = -2 WHERE rowid = :id AND status IN (0, 1)");
        QSqlQuery audit(database);
        audit.prepare("INSERT INTO SHIFT_AUDIT (shiftId, employeeId, action, reason, changedAt) "
                      "VALUES (:shift, :employee, 'cancel', :reason, :at)");
        for (int activeShiftId : shiftIds) {
            cancel.bindValue(":id", activeShiftId);
            if (!cancel.exec() || cancel.numRowsAffected() != 1)
                return fail(QString::fromUtf8("Không thể hủy ca làm liên quan đến yêu cầu nghỉ."));
            audit.bindValue(":shift", activeShiftId);
            audit.bindValue(":employee", employeeId);
            audit.bindValue(":reason", QString::fromUtf8("Nghỉ phép đã được duyệt"));
            audit.bindValue(":at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
            if (!audit.exec())
                return fail(audit.lastError().text());
        }
    }

    const QString message = approved
        ? QString::fromUtf8("Yêu cầu nghỉ ngày %1 đã được duyệt.").arg(displayDate(leaveDate))
        : QString::fromUtf8("Yêu cầu nghỉ ngày %1 đã bị từ chối.%2")
              .arg(displayDate(leaveDate), decisionReason.trimmed().isEmpty()
                   ? QString() : QString::fromUtf8(" Lý do: %1").arg(decisionReason.trimmed()));
    if (!Notification_Model::create(
            database, employeeId, approved ? "LEAVE_APPROVED" : "LEAVE_DECLINED",
            approved ? QString::fromUtf8("Yêu cầu nghỉ phép đã được duyệt")
                     : QString::fromUtf8("Yêu cầu nghỉ phép bị từ chối"),
            message, 0, requestId))
        return fail(QString::fromUtf8("Không thể tạo thông báo cho nhân viên."));

    if (!database.commit()) {
        if (error) *error = database.lastError().text();
        return false;
    }
    return true;
}
