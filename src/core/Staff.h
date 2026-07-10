#pragma once
#include "global.h"
#include "core/User.h"

class Staff: public User {
private:
	double hourSalary;
	double hourWork;
public:
    Staff(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add, QString phone, QString gender);
	double getSalary() const override;
};