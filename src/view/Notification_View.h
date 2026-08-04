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
    void reviewLeaveRequested(int notificationId, int leaveRequestId);
    void openManagerScheduleRequested(int notificationId);

private:
    QComboBox *filterBox = nullptr;
    QPushButton *markAllReadButton = nullptr;
    QLabel *emptyState = nullptr;
    QTableWidget *notificationTable = nullptr;
    bool managerMode = false;
};
