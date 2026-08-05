#include "global.h"
#include "view/Notification_View.h"

#include <QScrollArea>

namespace {
QString displayTime(const QDateTime &time) {
    return time.isValid() ? time.toLocalTime().toString("dd/MM/yyyy HH:mm") : "-";
}

QString accentColor(const NotificationInfo &notification) {
    if (notification.type == "STAFFING_SHORTAGE") return "#DC2626";
    if (notification.type.contains("APPROVED")) return "#16A34A";
    if (notification.type.contains("DECLINED") || notification.type.contains("CANCELLED"))
        return "#DC2626";
    if (notification.type.contains("SUBMITTED")) return "#D97706";
    return "#2563EB";
}

QString notificationTypeLabel(const NotificationInfo &notification) {
    if (notification.type == "STAFFING_SHORTAGE") return QString::fromUtf8("Cảnh báo");
    if (notification.type.startsWith("LEAVE_")) return QString::fromUtf8("Nghỉ phép");
    if (notification.type.startsWith("SHIFT_")) return QString::fromUtf8("Lịch làm");
    return QString::fromUtf8("Hệ thống");
}
}

Notification_View::Notification_View(QWidget *parent) : QWidget(parent) {
    setObjectName("NotificationView");
    setStyleSheet(
        "#NotificationView{background:#F8FAFC;}"
        "QComboBox{background:#FFFFFF;color:#334155;border:1px solid #CBD5E1;"
        "border-radius:6px;padding:7px 28px 7px 10px;min-width:110px;}"
        "QPushButton{background:#EFF6FF;color:#1D4ED8;border:1px solid #BFDBFE;"
        "border-radius:7px;padding:7px 10px;font-weight:700;}"
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
    markAllReadButton->setMinimumHeight(34);
    markAllReadButton->setStyleSheet(
        "QPushButton{background:#FFFFFF;color:#475569;border:1px solid #CBD5E1;}"
        "QPushButton:hover{background:#F1F5F9;}"
        "QPushButton:disabled{background:#F8FAFC;color:#94A3B8;border-color:#E2E8F0;}");
    header->addWidget(markAllReadButton);
    deleteReadButton = new QPushButton(QString::fromUtf8("Xóa đã đọc"), this);
    deleteReadButton->setMinimumHeight(34);
    deleteReadButton->setStyleSheet(
        "QPushButton{background:#FEF2F2;color:#B91C1C;border:1px solid #FECACA;}"
        "QPushButton:hover{background:#FEE2E2;}"
        "QPushButton:disabled{background:#FFF7F7;color:#FCA5A5;border-color:#FEE2E2;}");
    header->addWidget(deleteReadButton);
    layout->addLayout(header);

    notificationScroll = new QScrollArea(this);
    notificationScroll->setWidgetResizable(true);
    notificationScroll->setFrameShape(QFrame::NoFrame);
    notificationScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    notificationScroll->setStyleSheet("QScrollArea{background:transparent;}");
    notificationList = new QWidget(notificationScroll);
    notificationList->setStyleSheet("background:transparent;");
    notificationListLayout = new QVBoxLayout(notificationList);
    notificationListLayout->setContentsMargins(0, 0, 0, 0);
    notificationListLayout->setSpacing(10);
    notificationListLayout->setAlignment(Qt::AlignTop);
    notificationListLayout->addStretch();
    notificationScroll->setWidget(notificationList);
    layout->addWidget(notificationScroll, 1);

    emptyState = new QLabel(QString::fromUtf8("Bạn chưa có thông báo nào."), this);
    emptyState->setAlignment(Qt::AlignCenter);
    emptyState->setStyleSheet("color:#64748B;font-size:15px;padding:36px;");
    layout->addWidget(emptyState);

    connect(filterBox, qOverload<int>(&QComboBox::currentIndexChanged), this,
            [this](int) { emit filterChanged(currentFilter()); });
    connect(markAllReadButton, &QPushButton::clicked,
            this, &Notification_View::markAllReadRequested);
    connect(deleteReadButton, &QPushButton::clicked,
            this, &Notification_View::deleteReadRequested);
}

QString Notification_View::currentFilter() const {
    return filterBox->currentData().toString();
}

void Notification_View::setNotifications(const QList<NotificationInfo> &notifications,
                                         bool isManager) {
    managerMode = isManager;
    while (notificationListLayout->count() > 1) {
        QLayoutItem *item = notificationListLayout->takeAt(0);
        delete item->widget();
        delete item;
    }

    notificationScroll->setVisible(!notifications.isEmpty());
    emptyState->setVisible(notifications.isEmpty());
    markAllReadButton->setEnabled(!notifications.isEmpty());
    const bool hasRead = std::any_of(notifications.cbegin(), notifications.cend(),
                                     [](const NotificationInfo &notification) {
                                         return notification.status == "Read";
                                     });
    deleteReadButton->setEnabled(hasRead);

    for (const NotificationInfo &notification : notifications) {
        auto *card = new QFrame(notificationList);
        card->setObjectName("notificationCard");
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        card->setMaximumHeight(124);
        const bool unread = notification.status == "Unread";
        card->setStyleSheet(QString(
            "QFrame#notificationCard{background:%1;border:1px solid %2;border-radius:10px;}")
            .arg(unread ? "#F8FBFF" : "#FFFFFF", unread ? "#BFDBFE" : "#E2E8F0"));
        auto *cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(0, 0, 14, 0);
        cardLayout->setSpacing(12);

        auto *accent = new QFrame(card);
        accent->setFixedWidth(4);
        accent->setStyleSheet(QString("background:%1;border-top-left-radius:9px;"
                                      "border-bottom-left-radius:9px;")
                                  .arg(accentColor(notification)));
        cardLayout->addWidget(accent);

        auto *content = new QVBoxLayout();
        content->setContentsMargins(0, 12, 0, 12);
        content->setSpacing(4);
        auto *titleRow = new QHBoxLayout();
        titleRow->setSpacing(8);
        auto *type = new QLabel(notificationTypeLabel(notification), card);
        type->setStyleSheet(QString("background:%1;color:#FFFFFF;border-radius:9px;"
                                    "padding:2px 7px;font-size:11px;font-weight:700;")
                                .arg(accentColor(notification)));
        titleRow->addWidget(type, 0, Qt::AlignTop);
        auto *notificationTitle = new QLabel(notification.title, card);
        notificationTitle->setWordWrap(true);
        notificationTitle->setStyleSheet(QString("color:#0F172A;font-size:15px;font-weight:%1;")
                                             .arg(unread ? "800" : "700"));
        titleRow->addWidget(notificationTitle, 1);
        content->addLayout(titleRow);
        auto *message = new QLabel(notification.message, card);
        message->setWordWrap(true);
        message->setStyleSheet("color:#475569;font-size:13px;");
        content->addWidget(message);
        auto *time = new QLabel(displayTime(notification.createdAt), card);
        time->setStyleSheet("color:#94A3B8;font-size:12px;");
        content->addWidget(time);
        cardLayout->addLayout(content, 1);

        auto *side = new QVBoxLayout();
        side->setContentsMargins(0, 12, 0, 12);
        side->setSpacing(10);
        auto *status = new QLabel(unread ? QString::fromUtf8("Chưa đọc")
                                          : QString::fromUtf8("Đã đọc"), card);
        status->setAlignment(Qt::AlignCenter);
        status->setStyleSheet(unread
            ? "background:#DBEAFE;color:#1D4ED8;border-radius:10px;padding:3px 8px;font-size:11px;font-weight:700;"
            : "background:#F1F5F9;color:#64748B;border-radius:10px;padding:3px 8px;font-size:11px;font-weight:700;");
        side->addWidget(status, 0, Qt::AlignRight);

        auto *action = new QPushButton(card);
        action->setMinimumHeight(32);
        action->setMinimumWidth(118);
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
        } else if (managerMode && notification.type == "STAFFING_SHORTAGE") {
            action->setText(QString::fromUtf8("Mở xếp lịch"));
            connect(action, &QPushButton::clicked, this,
                    [this, notification] { emit openManagerScheduleRequested(notification.id); });
        } else if (unread) {
            action->setText(QString::fromUtf8("Đánh dấu đã đọc"));
            connect(action, &QPushButton::clicked, this,
                    [this, notification] { emit markReadRequested(notification.id); });
        } else {
            action->setText(QString::fromUtf8("Đã xem"));
            action->setEnabled(false);
        }
        action->setCursor(action->isEnabled() ? Qt::PointingHandCursor : Qt::ArrowCursor);
        side->addWidget(action, 0, Qt::AlignRight);
        cardLayout->addLayout(side);

        notificationListLayout->insertWidget(notificationListLayout->count() - 1, card);
    }
}
