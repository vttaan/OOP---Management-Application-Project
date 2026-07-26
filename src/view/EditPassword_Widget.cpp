#include "global.h"
#include "EditPassword_Widget.h"
#include <QScrollArea>

EditPassword_Widget::EditPassword_Widget(QWidget *parent) : QWidget(parent), isPanelOpen(false) {
    // This widget acts as the semi-transparent backdrop.
    // It covers the whole parent.
    this->hide();

    panelWidget = new QWidget(this);
    panelWidget->setStyleSheet("QWidget { background-color: white; border-top-left-radius: 16px; border-bottom-left-radius: 16px; } "
                               "QLabel { color: #1e293b; font-size: 14px; font-weight: 500; font-family: 'Segoe UI', Arial; } "
                               "QLineEdit { color: #8484a5; padding: 8px; border: 1px solid #cbd5e1; border-radius: 6px; font-size: 14px; } "
                               "QLineEdit:focus { border: 1px solid #3b82f6; }");

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setXOffset(-5);
    shadow->setYOffset(0);
    shadow->setColor(QColor(0, 0, 0, 80));
    panelWidget->setGraphicsEffect(shadow);

    QVBoxLayout *mainLayout = new QVBoxLayout(panelWidget);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(15);

    QLabel *lblTitle = new QLabel("Chỉnh sửa mật khẩu", panelWidget);
    lblTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #0f172a; border: none;");
    mainLayout->addWidget(lblTitle);

    // Scroll Area for the form content
    QScrollArea *scrollArea = new QScrollArea(panelWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; } QWidget#scrollContent { background: transparent; }");

    QWidget *scrollContent = new QWidget(scrollArea);
    scrollContent->setObjectName("scrollContent");
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSpacing(15);

    // Form section
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setVerticalSpacing(15);

    txtOldPassword = new password_LineEdit(scrollContent);
    txtNewPassword = new password_LineEdit(scrollContent);
    txtConfirmPassword = new password_LineEdit(scrollContent);

    formLayout->addRow("Mật khẩu cũ:", txtOldPassword);
    formLayout->addRow("Mật khẩu mới:", txtNewPassword);
    formLayout->addRow("Xác nhận mật khẩu mới:", txtConfirmPassword);

    scrollLayout->addLayout(formLayout);
    scrollLayout->addStretch();

    scrollContent->setLayout(scrollLayout);
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);

    // Buttons section
    QWidget *buttonsWidget = new QWidget(panelWidget);
    QHBoxLayout *buttonsLayout = new QHBoxLayout(buttonsWidget);
    buttonsLayout->setContentsMargins(0,0,0,0);

    QPushButton *btnCancel = new QPushButton("Hủy", buttonsWidget);
    btnCancel->setStyleSheet("QPushButton { background-color: #f1f5f9; border: 1px solid #cbd5e1; border-radius: 6px; padding: 10px 20px; font-weight: bold; color: #475569; } "
                             "QPushButton:hover { background-color: #e2e8f0; }");

    QPushButton *btnSave = new QPushButton("Lưu", buttonsWidget);
    btnSave->setStyleSheet("QPushButton { background-color: #003c71; border: none; border-radius: 6px; padding: 10px 20px; font-weight: bold; color: white; } "
                           "QPushButton:hover { background-color: #002b52; }");

    connect(btnCancel, &QPushButton::clicked, this, &EditPassword_Widget::onCancelClicked);
    connect(btnSave, &QPushButton::clicked, this, &EditPassword_Widget::onSaveClicked);

    buttonsLayout->addStretch();
    buttonsLayout->addWidget(btnCancel);
    buttonsLayout->addWidget(btnSave);
    mainLayout->addWidget(buttonsWidget);

    // Animation setup
    animation = new QPropertyAnimation(panelWidget, "pos", this);
    animation->setDuration(300);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    connect(animation, &QPropertyAnimation::finished, this, &EditPassword_Widget::onAnimationFinished);
}

void EditPassword_Widget::setInitialData() {
    txtOldPassword->setup();
    txtNewPassword->setup();
    txtConfirmPassword->setup();
}

void EditPassword_Widget::slideIn() {
    if (!parentWidget()) return;

    this->resize(parentWidget()->size());
    this->show();
    this->raise();

    int panelWidth = 450;
    panelWidget->resize(panelWidth, this->height());

    animation->setStartValue(QPoint(this->width(), 0));
    animation->setEndValue(QPoint(this->width() - panelWidth, 0));
    animation->start();

    isPanelOpen = true;
}

void EditPassword_Widget::slideOut() {
    if (!isPanelOpen) return;

    int panelWidth = 450;
    animation->setStartValue(panelWidget->pos());
    animation->setEndValue(QPoint(this->width(), 0));
    animation->start();

    isPanelOpen = false;
}

void EditPassword_Widget::onAnimationFinished() {
    if (!isPanelOpen) {
        this->hide();
    }
}

void EditPassword_Widget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    // Draw semi-transparent dark backdrop
    painter.fillRect(rect(), QColor(0, 0, 0, 100));
}

void EditPassword_Widget::mousePressEvent(QMouseEvent *event) {
    // If clicked outside the panel widget, close it
    if (!panelWidget->geometry().contains(event->pos())) {
        slideOut();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void EditPassword_Widget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (isPanelOpen) {
        int panelWidth = 450;
        panelWidget->setGeometry(this->width() - panelWidth, 0, panelWidth, this->height());
    } else {
        panelWidget->setGeometry(this->width(), 0, 450, this->height());
    }
}

void EditPassword_Widget::onSaveClicked() {
    emit saveRequested(txtOldPassword->text(), txtNewPassword->text());
}

void EditPassword_Widget::onCancelClicked() {
    slideOut();
}
