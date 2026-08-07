#include "EmployeeDetails_Dialog.h"
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QDir>
#include <QCoreApplication>

EmployeeDetails_Dialog::EmployeeDetails_Dialog(User* emp, QWidget* parent)
    : QDialog(parent)
{
    if (!emp) return;
    setWindowTitle("Thông tin chi tiết nhân viên");
    if (parent && parent->window()) {
        QSize winSize = parent->window()->size();
        setFixedSize(winSize.width() * 0.7, winSize.height() * 0.7);
    } else {
        setFixedSize(800, 600);
    }
    setModal(true);
    
    // Modern styling with rounded corners and light blue borders
    this->setStyleSheet("QDialog { background-color: #ffffff; border: 1px solid #BAE6FD; border-radius: 12px; }");
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    
    setupUi(emp);
}

void EmployeeDetails_Dialog::setupUi(User* emp)
{
    // A container widget to hold the background so we can apply rounded corners cleanly
    QWidget* container = new QWidget(this);
    container->setStyleSheet("QWidget { background-color: #ffffff; border: 2px solid #BAE6FD; border-radius: 16px; }");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->addWidget(container);

    QVBoxLayout* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(24, 24, 24, 24);
    containerLayout->setSpacing(24);

    // Header with Title and Close Button
    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* titleLabel = new QLabel("Hồ Sơ Nhân Viên");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #0369A1; border: none;");
    
    QPushButton* closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(32, 32);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet("QPushButton { background: #F1F5F9; color: #64748B; font-size: 16px; font-weight: bold; border: none; border-radius: 16px; }"
                            "QPushButton:hover { background: #FEE2E2; color: #EF4444; }");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeBtn);
    containerLayout->addLayout(headerLayout);

    // Divider
    QFrame* divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("border: none; background-color: #E2E8F0; max-height: 1px;");
    containerLayout->addWidget(divider);

    // Content: 2 Columns
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(32);

    // ---- Left Column: Avatar & Name ----
    QVBoxLayout* leftCol = new QVBoxLayout();
    leftCol->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    leftCol->setSpacing(10);
    
    QLabel* avatarLabel = new QLabel();
    avatarLabel->setFixedSize(160, 160);
    avatarLabel->setStyleSheet("border: none;");
    QPixmap avatarPix = getRoundedAvatar(emp->getAvatarPath(), emp->getName(), emp->getRole(), 160);
    avatarLabel->setPixmap(avatarPix);
    
    QLabel* nameLabel = new QLabel(emp->getName());
    nameLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #0F172A; border: none;");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);

    QLabel* roleBadge = new QLabel(getVietnameseRole(emp->getRole()));
    roleBadge->setAlignment(Qt::AlignCenter);
    roleBadge->setFixedHeight(32);
    roleBadge->setStyleSheet("background-color: #E0F2FE; color: #0284C7; border: none; border-radius: 16px; font-size: 15px; font-weight: bold; padding: 0 20px;");

    leftCol->addWidget(avatarLabel, 0, Qt::AlignHCenter);
    leftCol->addWidget(nameLabel, 0, Qt::AlignHCenter);
    leftCol->addWidget(roleBadge, 0, Qt::AlignHCenter);
    
    contentLayout->addLayout(leftCol, 1);

    // ---- Right Column: Info Form ----
    QFormLayout* form = new QFormLayout();
    form->setSpacing(24);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // Custom formatting for rows
    addRowToForm(form, "Mã Nhân Viên:", "NV-" + QString::number(emp->getIdEmployee()), true);
    addRowToForm(form, "Căn Cước (CCCD):", emp->getIdentityID());


    addRowToForm(form, "Giới Tính:", emp->getGender());
    addRowToForm(form, "Ngày Sinh:", emp->getDOB());
    addRowToForm(form, "Số Điện Thoại:", emp->getPhoneNum());
    addRowToForm(form, "Địa Chỉ:", emp->getAddress());
    
    QString payType = emp->getIsFixedSalary() ? "Cố định (Theo tháng)" : "Theo giờ";
    addRowToForm(form, "Loại Lương:", payType);

    QLocale locale(QLocale::Vietnamese, QLocale::Vietnam);
    QString salaryStr = locale.toString((long long)emp->getBaseSalary()) + (emp->getIsFixedSalary() ? " vnđ / tháng" : " vnđ / giờ");
    addRowToForm(form, "Mức Lương:", salaryStr, true);
    
    // Bỏ hiển thị đường dẫn ảnh theo yêu cầu

    contentLayout->addLayout(form, 2);
    
    containerLayout->addLayout(contentLayout);
    containerLayout->addStretch();
}

void EmployeeDetails_Dialog::addRowToForm(QFormLayout* form, const QString& labelText, const QString& valueText, bool isHighlight)
{
    QLabel* lbl = new QLabel(labelText);
    lbl->setStyleSheet("font-size: 16px; font-weight: bold; color: #64748B; border: none;");
    
    QLabel* val = new QLabel(valueText);
    val->setWordWrap(true);
    if (isHighlight) {
        val->setStyleSheet("font-size: 17px; font-weight: bold; color: #0F172A; border: none;");
    } else {
        val->setStyleSheet("font-size: 16px; color: #334155; border: none;");
    }
    
    form->addRow(lbl, val);
}

QPixmap EmployeeDetails_Dialog::getRoundedAvatar(const QString& avatarPath, const QString& name, const QString& role, int size)
{
    QPixmap target(size, size);
    target.fill(Qt::transparent);
    
    QPainter painter(&target);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);

    QPixmap src;
    if (!avatarPath.isEmpty()) {
        if (avatarPath.startsWith(":/")) {
            src.load(avatarPath);
        } else {
            QDir appDir(QCoreApplication::applicationDirPath());
            appDir.cdUp(); // build
            appDir.cdUp(); // project root
            QString fullPath = appDir.filePath("resources/avatars/") + avatarPath;
            if (QFile::exists(fullPath))
                src.load(fullPath);
        }
    }

    if (!src.isNull()) {
        QPixmap scaled = src.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        // Center the scaled pixmap
        int x = (size - scaled.width()) / 2;
        int y = (size - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);
    } else {
        QString bgColor = role.contains("Manager", Qt::CaseInsensitive) ? "#9333EA" : "#1A73E8";
        painter.fillRect(0, 0, size, size, QColor(bgColor));
        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setPixelSize(size / 2);
        font.setBold(true);
        painter.setFont(font);
        QString initial = name.isEmpty() ? "?" : QString(name[0]).toUpper();
        painter.drawText(QRect(0, 0, size, size), Qt::AlignCenter, initial);
    }
    return target;
}

QString EmployeeDetails_Dialog::getVietnameseRole(const QString& roleName)
{
    if (roleName == "Manager") return "Quản lý";
    if (roleName == "Admin") return "Quản trị viên";
    if (roleName == "Cashier") return "Thu ngân";
    if (roleName == "HallStaff") return "Nhân viên sảnh";
    if (Config::canonicalRoleName(roleName) == "KitchenAssistant") return "Phụ bếp";
    return "Nhân viên";
}

void EmployeeDetails_Dialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void EmployeeDetails_Dialog::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - dragPosition);
        event->accept();
    }
}
