#pragma once
#include "global.h"
#include "core/User.h"

class Staff: public User {
private:
    double hourSalary = 0;
    double hourWork = 0;
public:
    Staff(QString r, short int idEmp, QString ava, QString idCit, QString n
          , QString d, QString add, QString phone, QString gender, int baseSalary);
    double getSalary() const override;
    double getHourWork();
};