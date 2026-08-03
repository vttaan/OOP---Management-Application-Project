#pragma once
#include "global.h"
#include "core/User.h"

class Staff: public User {
private:
    double hourSalary = 0;
    double hourWork = 0;
    bool isFixedEmployee;
public:
    Staff(QString r, short int idEmp, QString ava, QString idCit, QString n
          , QString d, QString add, QString phone, QString gender, int baseSalary, bool isFixedEmployee);
    double getSalary() const override;
    double getBaseSalary() const override;
    double getHourWork();

    void setBaseSalary(double salary) override;
    void setFixedEmployee(bool isFixed) override;
    bool getIsFixedEmployee() const override { return isFixedEmployee; }
    virtual double getAllowence() const = 0;
    void setAllowence() override = 0;
    void setAllowenceValue(double allowance) override = 0;
};