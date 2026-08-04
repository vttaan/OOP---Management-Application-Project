#pragma once
#include "global.h"
#include "core/UserPrototypeRegistry.h"
class UserFactory
{
public:
    static User *createContainsUser(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add,
                                    QString phone, QString gender, int baseSalary, bool isFixed = false, double allowance = 0);
    static User *createNewUser(QString r, QString ava, QString idCit, QString n, QString d, QString add,
                               QString phone, QString gender, int baseSalary, bool isFixedSalary = false, double allowance = 0);
};
