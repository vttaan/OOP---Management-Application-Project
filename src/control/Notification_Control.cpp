#include "global.h"
#include "control/Notification_Control.h"
#include "utils/SessionManage.h"
#include "view/Notification_View.h"

Notification_Control::Notification_Control(QObject *parent) : QObject(parent) {}

void Notification_Control::setView(Notification_View *newView) {
    view = newView;
    if (!view) return;
    connect(view, &Notification_View::filterChanged, this,
            [this](const QString &) { load(); });
    connect(view, &Notification_View::markReadRequested,
            this, &Notification_Control::markRead);
    connect(view, &Notification_View::markAllReadRequested,
            this, &Notification_Control::markAllRead);
    connect(view, &Notification_View::reviewLeaveRequested,
            this, &Notification_Control::reviewLeaveRequest);
    connect(view, &Notification_View::openManagerScheduleRequested,
            this, &Notification_Control::openManagerSchedule);
}

void Notification_Control::load() {
    User *user = SessionManager::getInstance()->getCurrentUser();
    if (!view || !user) return;
    const bool managerMode = user->getRole() == "Manager" || user->getRole() == "Admin";
    view->setNotifications(notificationModel.getNotifications(user->getIdEmployee(),
                                                               view->currentFilter()),
                           managerMode);
    refreshUnreadCount();
}

void Notification_Control::refreshUnreadCount() {
    User *user = SessionManager::getInstance()->getCurrentUser();
    emit unreadCountChanged(user ? notificationModel.getUnreadCount(user->getIdEmployee()) : 0);
}

void Notification_Control::markRead(int notificationId) {
    User *user = SessionManager::getInstance()->getCurrentUser();
    if (!user) return;
    notificationModel.markAsRead(notificationId, user->getIdEmployee());
    load();
}

void Notification_Control::markAllRead() {
    User *user = SessionManager::getInstance()->getCurrentUser();
    if (!user) return;
    notificationModel.markAllAsRead(user->getIdEmployee());
    load();
}

void Notification_Control::reviewLeaveRequest(int notificationId, int leaveRequestId) {
    User *user = SessionManager::getInstance()->getCurrentUser();
    if (!user || (user->getRole() != "Manager" && user->getRole() != "Admin")) return;
    const auto choice = QMessageBox::question(
        view, QString::fromUtf8("Xử lý yêu cầu nghỉ phép"),
        QString::fromUtf8("Duyệt yêu cầu nghỉ phép này? Chọn No để từ chối."),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
    if (choice == QMessageBox::Cancel) return;

    bool accepted = false;
    const QString reason = QInputDialog::getText(
        view, QString::fromUtf8("Lý do quyết định"),
        choice == QMessageBox::Yes ? QString::fromUtf8("Ghi chú phê duyệt (không bắt buộc):")
                                  : QString::fromUtf8("Lý do từ chối:"),
        QLineEdit::Normal, {}, &accepted).trimmed();
    if (!accepted) return;

    QString error;
    if (!leaveRequestModel.decideLeaveRequest(leaveRequestId, user->getIdEmployee(),
                                               choice == QMessageBox::Yes, reason, &error)) {
        QMessageBox::warning(view, QString::fromUtf8("Không thể xử lý"), error);
        return;
    }
    notificationModel.markLeaveRequestReviewedByRequest(
        leaveRequestId, choice == QMessageBox::Yes);
    load();
    emit leaveRequestDecisionCompleted();
}

void Notification_Control::openManagerSchedule(int notificationId) {
    User *user = SessionManager::getInstance()->getCurrentUser();
    if (user) notificationModel.markAsRead(notificationId, user->getIdEmployee());
    refreshUnreadCount();
    emit openManagerScheduleRequested();
}
