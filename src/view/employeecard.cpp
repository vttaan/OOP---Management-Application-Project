#include "employeecard.h"
#include "ui_employeecard.h"

EmployeeCard::EmployeeCard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EmployeeCard)
{
    ui->setupUi(this);
}

EmployeeCard::~EmployeeCard()
{
    delete ui;
}
void EmployeeCard::setData(QString avatarPath, QString name, QString role, QString email, QString phone)
{
    ui->lblName->setText(name);
    ui->lblRole->setText(role);
    ui->lblEmail->setText(email);
    ui->lblPhone->setText(phone);

    ui->lblAvatar->setText("");

    QString css = QString("background-image: url(%1); "
                          "background-position: center; "
                          "background-repeat: no-repeat; "
                          "border-radius: 40px;").arg(avatarPath);
    ui->lblAvatar->setStyleSheet(css);
    QPixmap originalPixmap(avatarPath);

    QPixmap scaledPixmap = originalPixmap.scaled(ui->lblAvatar->size(),
                                                 Qt::KeepAspectRatioByExpanding,
                                                 Qt::SmoothTransformation);

    ui->lblAvatar->setPixmap(scaledPixmap);
}