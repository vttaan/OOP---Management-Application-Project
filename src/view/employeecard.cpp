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
                           const QString& role, const QString& email,
                           const QString& phone, const QString& id, 
                           const QString& dob, const QString& gender)
{
    ui->lblName->setText(name);
    ui->lblRole->setText(role);
    ui->lblEmail->setText("✉  " + email);
    ui->lblPhone->setText("📞  " + phone);
    ui->lblID->setText("🆔  Mã số: " + id);
    ui->lblDOB->setText("🎂  Ngày sinh: " + dob);
    ui->lblGender->setText("🚻  Giới tính: " + gender);
    QPixmap pix(avatarPath);
    if (!pix.isNull()) {
        ui->lblAvatar->setPixmap(
            pix.scaled(ui->lblAvatar->size(),
                       Qt::KeepAspectRatioByExpanding,
                       Qt::SmoothTransformation));
        ui->lblAvatar->setText("");
    } else {
        QString initial = name.isEmpty() ? "?" : QString(name[0]).toUpper();
        QString bgColor = (role.contains("Manager", Qt::CaseInsensitive))
                              ? "#9333ea" : "#1a73e8";
        ui->lblAvatar->setText(initial);
        ui->lblAvatar->setStyleSheet(
            QString("background-color: %1; color: white; "
                    "border-radius: 30px; font-size: 24px; font-weight: bold;")
                .arg(bgColor));
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
