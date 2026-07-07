#include "EditProfile_Widget.h"
#include <QFormLayout>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDebug>
#include <QGraphicsDropShadowEffect>

EditProfile_Widget::EditProfile_Widget(QWidget *parent) : QWidget(parent), isPanelOpen(false) {
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
    mainLayout->setContentsMargins(30, 40, 30, 40);
    mainLayout->setSpacing(20);
    
    QLabel *lblTitle = new QLabel("Chỉnh sửa thông tin", panelWidget);
    lblTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #0f172a; border: none;");
    mainLayout->addWidget(lblTitle);
    
    // Avatar section
    QWidget *avatarWidget = new QWidget(panelWidget);
    QHBoxLayout *avatarLayout = new QHBoxLayout(avatarWidget);
    avatarLayout->setContentsMargins(0,0,0,0);
    
    lblAvatarPreview = new QLabel(avatarWidget);
    lblAvatarPreview->setFixedSize(100, 100);
    lblAvatarPreview->setStyleSheet("border: 1px solid #e2e8f0; border-radius: 8px; background-color: #f1f5f9;");
    lblAvatarPreview->setAlignment(Qt::AlignCenter);
    
    QPushButton *btnChangeAvatar = new QPushButton("Thay đổi ảnh", avatarWidget);
    btnChangeAvatar->setStyleSheet("QPushButton { background-color: #f8fafc; border: 1px solid #cbd5e1; border-radius: 6px; padding: 8px 16px; font-weight: bold; color: #475569; } "
                                   "QPushButton:hover { background-color: #f1f5f9; border-color: #94a3b8; }");
    connect(btnChangeAvatar, &QPushButton::clicked, this, &EditProfile_Widget::onChangeAvatarClicked);
    
    avatarLayout->addWidget(lblAvatarPreview);
    avatarLayout->addWidget(btnChangeAvatar);
    avatarLayout->addStretch();
    mainLayout->addWidget(avatarWidget);
    
    // Form section
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setVerticalSpacing(15);
    
    txtName = new QLineEdit(panelWidget);
    txtDob = new QLineEdit(panelWidget);
    txtAddress = new QLineEdit(panelWidget);
    txtPhone = new QLineEdit(panelWidget);
    txtCitizenId = new QLineEdit(panelWidget);
    
    formLayout->addRow("Họ và tên:", txtName);
    formLayout->addRow("Ngày sinh:", txtDob);
    formLayout->addRow("Địa chỉ:", txtAddress);
    formLayout->addRow("Số điện thoại:", txtPhone);
    formLayout->addRow("CMND/CCCD:", txtCitizenId);
    
    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
    
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
                           
    connect(btnCancel, &QPushButton::clicked, this, &EditProfile_Widget::onCancelClicked);
    connect(btnSave, &QPushButton::clicked, this, &EditProfile_Widget::onSaveClicked);
    
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(btnCancel);
    buttonsLayout->addWidget(btnSave);
    mainLayout->addWidget(buttonsWidget);
    
    // Animation setup
    animation = new QPropertyAnimation(panelWidget, "pos", this);
    animation->setDuration(300);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    connect(animation, &QPropertyAnimation::finished, this, &EditProfile_Widget::onAnimationFinished);
}

void EditProfile_Widget::setInitialData(const QString& name, const QString& dob, const QString& address, const QString& phone, const QString& citizenId, const QString& avatarPath) {
    txtName->setText(name);
    txtDob->setText(dob);
    txtAddress->setText(address);
    txtPhone->setText(phone);
    txtCitizenId->setText(citizenId);
    currentAvatarPath = avatarPath;
    
    // Load avatar preview
    QDir appDir(QCoreApplication::applicationDirPath());
    QString folderPath = appDir.filePath("avatars");
    
    QPixmap pixmap(folderPath + "/" + avatarPath);
    if(avatarPath.isEmpty() || !QFileInfo::exists(folderPath + "/" + avatarPath) || pixmap.isNull()) {
        pixmap.load(":/images/avatarSample.png");
    }
    
    if(!pixmap.isNull()) {
        QPixmap scaled = pixmap.scaled(100, 100, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        lblAvatarPreview->setPixmap(scaled);
    }
}

void EditProfile_Widget::slideIn() {
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

void EditProfile_Widget::slideOut() {
    if (!isPanelOpen) return;
    int panelWidth = 450;
    animation->setStartValue(panelWidget->pos());
    animation->setEndValue(QPoint(this->width(), 0));
    animation->start();
    
    isPanelOpen = false;
}

void EditProfile_Widget::onAnimationFinished() {
    if (!isPanelOpen) {
        this->hide();
    }
}

void EditProfile_Widget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    // Draw semi-transparent dark backdrop
    painter.fillRect(rect(), QColor(0, 0, 0, 100));
}

void EditProfile_Widget::mousePressEvent(QMouseEvent *event) {
    // If clicked outside the panel widget, close it
    if (!panelWidget->geometry().contains(event->pos())) {
        slideOut();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void EditProfile_Widget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (isPanelOpen) {
        int panelWidth = 450;
        panelWidget->setGeometry(this->width() - panelWidth, 0, panelWidth, this->height());
    } else {
        panelWidget->setGeometry(this->width(), 0, 450, this->height());
    }
}

void EditProfile_Widget::onSaveClicked() {
    emit saveRequested(txtName->text(), txtDob->text(), txtAddress->text(), txtPhone->text(), txtCitizenId->text(), currentAvatarPath);
}

void EditProfile_Widget::onCancelClicked() {
    slideOut();
}

void EditProfile_Widget::onChangeAvatarClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Chọn ảnh đại diện", "", "Images (*.png *.xpm *.jpg *.jpeg)");
    if (!filePath.isEmpty()) {
        currentAvatarPath = filePath;
        
        // Update preview
        QPixmap pixmap(filePath);
        if (!pixmap.isNull()) {
            QPixmap scaled = pixmap.scaled(100, 100, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            lblAvatarPreview->setPixmap(scaled);
        }
    }
}
