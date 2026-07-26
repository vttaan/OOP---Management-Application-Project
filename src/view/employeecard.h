#include "global.h"
#ifndef EMPLOYEECARD_H
#define EMPLOYEECARD_H

namespace Ui {
class EmployeeCard;
}

class EmployeeCard : public QWidget
{
    Q_OBJECT

public:
    explicit EmployeeCard(QWidget *parent = nullptr);
    ~EmployeeCard();
    void setData(const QString& avatarPath, const QString& name, const QString& role, const QString& email, const QString& phone);
    void setStatus(bool isWorking);
private:
    Ui::EmployeeCard *ui;

};

#endif
