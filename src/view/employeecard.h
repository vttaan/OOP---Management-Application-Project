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
void setData(QString avatarPath,QString name, QString role, QString email, QString phone);
private:
    Ui::EmployeeCard *ui;
};

#endif
