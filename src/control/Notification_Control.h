#pragma once

#include "global.h"
#include "model/LeaveRequest_Model.h"
#include "model/Notification_Model.h"

class Notification_View;

class Notification_Control : public QObject {
    Q_OBJECT
public:
    explicit Notification_Control(QObject *parent = nullptr);
    void setView(Notification_View *view);
    void load();
    void refreshUnreadCount();

signals:
    void unreadCountChanged(int count);
    void openManagerScheduleRequested();

private slots:
    void markRead(int notificationId);
    void markAllRead();
    void reviewLeaveRequest(int notificationId, int leaveRequestId);
    void openManagerSchedule(int notificationId);

private:
    Notification_View *view = nullptr;
    Notification_Model notificationModel;
    LeaveRequest_Model leaveRequestModel;
};
