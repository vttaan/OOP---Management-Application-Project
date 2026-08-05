#include "global.h"
#include "view/Notification_View.h"

namespace {
QString displayTime(const QDateTime &time) {
    return time.isValid() ? time.toLocalTime().toString("dd/MM/yyyy HH:mm") : "-";
}

QColor statusColor(const NotificationInfo &notification) {
    if (notification.type.contains("APPROVED")) return QColor("#15803D");
    if (notification.type.contains("DECLINED") || notification.type.contains("CANCELLED"))
        return QColor("#B91C1C");
    if (notification.type.contains("SUBMITTED")) return QColor("#B45309");
    return QColor("#1D4ED8");
}
}

Notification_View::Notification_View(QWidget *parent) : QWidget(parent) {
    setObjectName("NotificationView");
    setStyleSheet(
        "#NotificationView{background:#F8FAFC;}"
        "QComboBox{background:#FFFFFF;color:#334155;border:1px solid #CBD5E1;"
        "border-radius:6px;padding:7px 28px 7px 10px;min-width:110px;}"
        "QPushButton{background:#EFF6FF;color:#1D4ED8;border:1px solid #BFDBFE;"
        "border-radius:6px;padding:7px 10px;font-weight:700;}"
        "QPushButton:hover{background:#DBEAFE;}"
        "QPushButton:disabled{background:#F1F5F9;color:#64748B;border-color:#E2E8F0;}");
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(14);

    auto *header = new QHBoxLayout();
    auto *icon = new QLabel(this);
    icon->setPixmap(QIcon(":/images/notification-bell.svg").pixmap(30, 30));
    header->addWidget(icon);
    auto *title = new QLabel(QString::fromUtf8("Thông báo"), this);
    title->setStyleSheet("font-size:24px;font-weight:800;color:#0F172A;");
    header->addWidget(title);
    header->addStretch();
    filterBox = new QComboBox(this);
    filterBox->addItem(QString::fromUtf8("Tất cả"), "All");
    filterBox->addItem(QString::fromUtf8("Chưa đọc"), "Unread");
    filterBox->addItem(QString::fromUtf8("Lịch làm"), "Shift");
    filterBox->addItem(QString::fromUtf8("Nghỉ phép"), "Leave");
    header->addWidget(filterBox);
    markAllReadButton = new QPushButton(QString::fromUtf8("Đánh dấu tất cả đã đọc"), this);
    header->addWidget(markAllReadButton);
    markAllReadButton->setMinimumHeight(34);
    layout->addLayout(header);

    notificationTable = new QTableWidget(this);
    notificationTable->setColumnCount(4);
    notificationTable->setHorizontalHeaderLabels({QString::fromUtf8("Thông báo"),
                                                   QString::fromUtf8("Thời gian"),
                                                   QString::fromUtf8("Trạng thái"),
                                                   QString::fromUtf8("Thao tác")});
    notificationTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    notificationTable->setSelectionMode(QAbstractItemView::NoSelection);
    notificationTable->verticalHeader()->setVisible(false);
    notificationTable->horizontalHeader()->setStretchLastSection(false);
    notificationTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    notificationTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    notificationTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    notificationTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    notificationTable->setStyleSheet(
        "QTableWidget{background:#FFFFFF;border:1px solid #E2E8F0;border-radius:10px;}"
        "QHeaderView::section{background:#F8FAFC;color:#475569;border:none;padding:10px;font-weight:700;}"
        "QTableWidget::item{border-bottom:1px solid #E2E8F0;padding:8px;color:#1E293B;}");
    layout->addWidget(notificationTable, 1);

    emptyState = new QLabel(QString::fromUtf8("Bạn chưa có thông báo nào."), this);
    emptyState->setAlignment(Qt::AlignCenter);
    emptyState->setStyleSheet("color:#64748B;font-size:15px;padding:36px;");
    layout->addWidget(emptyState);

    connect(filterBox, qOverload<int>(&QComboBox::currentIndexChanged), this,
            [this](int) { emit filterChanged(currentFilter()); });
    connect(markAllReadButton, &QPushButton::clicked,
            this, &Notification_View::markAllReadRequested);
}

QString Notification_View::currentFilter() const {
    return filterBox->currentData().toString();
}

void Notification_View::setNotifications(const QList<NotificationInfo> &notifications,
                                         bool isManager) {
    managerMode = isManager;
    notificationTable->setRowCount(0);
    notificationTable->setVisible(!notifications.isEmpty());
    emptyState->setVisible(notifications.isEmpty());
    markAllReadButton->setEnabled(!notifications.isEmpty());

    for (int row = 0; row < notifications.size(); ++row) {
        const NotificationInfo &notification = notifications[row];
        notificationTable->insertRow(row);
        notificationTable->setRowHeight(row, 68);

        auto *message = new QTableWidgetItem(
            QString("%1\n%2").arg(notification.title, notification.message));
        QFont messageFont = message->font();
        messageFont.setBold(notification.status == "Unread");
        message->setFont(messageFont);
        message->setData(Qt::UserRole, notification.id);
        notificationTable->setItem(row, 0, message);
        notificationTable->setItem(row, 1, new QTableWidgetItem(displayTime(notification.createdAt)));

        auto *status = new QTableWidgetItem(
            notification.status == "Unread" ? QString::fromUtf8("Chưa đọc")
                                            : QString::fromUtf8("Đã đọc"));
        status->setForeground(statusColor(notification));
        notificationTable->setItem(row, 2, status);

        auto *action = new QPushButton(notificationTable);
        action->setMinimumHeight(32);
        action->setMinimumWidth(112);
        const bool isPendingLeaveRequest =
            notification.relatedLeaveRequestStatus.isEmpty() ||
            notification.relatedLeaveRequestStatus == "Pending";
        if (managerMode && notification.type == "LEAVE_SUBMITTED" &&
            notification.relatedLeaveRequestId > 0 && isPendingLeaveRequest) {
            action->setText(QString::fromUtf8("Duyệt yêu cầu"));
            connect(action, &QPushButton::clicked, this,
                    [this, notification] {
                        emit reviewLeaveRequested(notification.id,
                                                  notification.relatedLeaveRequestId);
                    });
        } else if (managerMode && notification.relatedLeaveRequestId > 0 &&
                   (notification.type == "LEAVE_APPROVED" ||
                    notification.relatedLeaveRequestStatus == "Approved")) {
            action->setText(QString::fromUtf8("Đã duyệt"));
            action->setEnabled(false);
        } else if (managerMode && notification.relatedLeaveRequestId > 0 &&
                   (notification.type == "LEAVE_DECLINED" ||
                    notification.relatedLeaveRequestStatus == "Declined")) {
            action->setText(QString::fromUtf8("Đã từ chối"));
            action->setEnabled(false);
        } else if (managerMode && notification.type == "SHIFT_SUBMITTED") {
            action->setText(QString::fromUtf8("Mở xếp lịch"));
            connect(action, &QPushButton::clicked, this,
                    [this, notification] { emit openManagerScheduleRequested(notification.id); });
        } else if (notification.status == "Unread") {
            action->setText(QString::fromUtf8("Đánh dấu đã đọc"));
            connect(action, &QPushButton::clicked, this,
                    [this, notification] { emit markReadRequested(notification.id); });
        } else {
            action->setText(QString::fromUtf8("Đã xem"));
            action->setEnabled(false);
        }
        action->setCursor(Qt::PointingHandCursor);
        notificationTable->setCellWidget(row, 3, action);
    }
}
