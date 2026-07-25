#pragma once
#include "global.h"
#include "core/Manager.h"
#include "core/Staff.h"
class UserFactory
{
public:
  static User *createContainsUser(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add,
                                  QString phone, QString gender, int baseSalary);
  static User *createNewUser(QString r, QString ava, QString idCit, QString n, QString d, QString add,
                             QString phone, QString gender, int baseSalary);
};
