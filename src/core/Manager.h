#pragma once
#include "global.h"
#include "core/User.h"
class Manager : public User {
private:
	double fixSalary;
	short int dayWork;
public:
    Manager(QString r, short int idEmp, QString ava, QString idCit, QString n
            , QString d, QString add, QString phone, QString gender, int baseSalary);
	double getSalary() const override;
};