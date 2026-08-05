#pragma once

#include "global.h"
#include "utils/NotificationDTOs.h"

class Notification_View : public QWidget {
    Q_OBJECT
public:
    explicit Notification_View(QWidget *parent = nullptr);
    void setNotifications(const QList<NotificationInfo> &notifications,
                          bool managerMode);
    QString currentFilter() const;

signals:
    void filterChanged(const QString &filter);
    void markReadRequested(int notificationId);
    void markAllReadRequested();
    void deleteReadRequested();
    void reviewLeaveRequested(int notificationId, int leaveRequestId);
    void openManagerScheduleRequested(int notificationId);

private:
    QComboBox *filterBox = nullptr;
    QPushButton *markAllReadButton = nullptr;
    QPushButton *deleteReadButton = nullptr;
    QLabel *emptyState = nullptr;
    QScrollArea *notificationScroll = nullptr;
    QWidget *notificationList = nullptr;
    QVBoxLayout *notificationListLayout = nullptr;
    bool managerMode = false;
};
