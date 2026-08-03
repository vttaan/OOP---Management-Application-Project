#include "global.h"
#include "employeecard.h"
#include "ui_employeecard.h"
EmployeeCard::EmployeeCard(QWidget *parent)
    : QWidget(parent), ui(new Ui::EmployeeCard)
{
    ui->setupUi(this);
}
EmployeeCard::~EmployeeCard() { delete ui; }
void EmployeeCard::setData(const QString& avatarPath, const QString& name,
                           const QString& role, const QString& phone,
                           const QString& id, const QString& dob,
                           const QString& gender)
{
    ui->lblName->setText(name);
    ui->lblRole->setText(role);
    ui->lblPhone->setText("📞  " + phone);
    ui->lblID->setText("🆔  Mã số: " + id);
    ui->lblDOB->setText("🎂  Ngày sinh: " + dob);

    QString viGender = gender;
    if (gender.compare("Male", Qt::CaseInsensitive) == 0) viGender = "Nam";
    else if (gender.compare("Female", Qt::CaseInsensitive) == 0) viGender = "Nữ";
    else if (gender.compare("Other", Qt::CaseInsensitive) == 0) viGender = "Khác";

    ui->lblGender->setText("🚻  Giới tính: " + viGender);
    QPixmap pix(avatarPath);
    if (!pix.isNull()) {
        QSize size = ui->lblAvatar->size();
        QPixmap scaledPix = pix.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPixmap circularPix(size);
        circularPix.fill(Qt::transparent);
        QPainter painter(&circularPix);
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addEllipse(circularPix.rect());
        painter.setClipPath(path);
        
        int x = (circularPix.width() - scaledPix.width()) / 2;
        int y = (circularPix.height() - scaledPix.height()) / 2;
        painter.drawPixmap(x, y, scaledPix);
        
        ui->lblAvatar->setPixmap(circularPix);
        ui->lblAvatar->setStyleSheet("background: transparent; border: none;");
        ui->lblAvatar->setText("");
    } else {
        QString initial = name.isEmpty() ? "?" : QString(name[0]).toUpper();
        QString bgColor = (role.contains("Manager", Qt::CaseInsensitive))
                              ? "#9333ea" : "#1a73e8";

        QSize size(50, 50);
        QPixmap circularPix(size);
        circularPix.fill(Qt::transparent);
        QPainter painter(&circularPix);
        painter.setRenderHint(QPainter::Antialiasing);

        QPainterPath path;
        path.addEllipse(circularPix.rect());
        painter.setClipPath(path);
        
        painter.fillRect(circularPix.rect(), QColor(bgColor));
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 20, QFont::Bold));
        painter.drawText(circularPix.rect(), Qt::AlignCenter, initial);

        ui->lblAvatar->setPixmap(circularPix);
        ui->lblAvatar->setStyleSheet("background: transparent; border: none;");
        ui->lblAvatar->setText("");
    }
}
void EmployeeCard::setStatus(bool isWorking)
{
    if (isWorking) {
        ui->btnStatus->setText("● Đang làm việc");
        ui->btnStatus->setStyleSheet(
            "QPushButton { background-color: #e6f4ea; color: #1e8e3e; "
            "border: none; border-radius: 14px; padding: 2px 10px; }");
    } else {
        ui->btnStatus->setText("● Đang nghỉ");
        ui->btnStatus->setStyleSheet(
            "QPushButton { background-color: #fdecea; color: #c5221f; "
            "border: none; border-radius: 14px; padding: 2px 10px; }");
    }
}
