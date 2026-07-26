#include "global.h"
#include "core/Staff.h"

Staff::Staff(QString r, short int idEmp, QString ava, QString idCit, QString n,
             QString d, QString add, QString phone, QString gender, int baseSalary)
    : User(r, idEmp, ava, idCit, n, d, add, phone, gender)
    , hourSalary(baseSalary) {}

double Staff::getSalary() const { return hourSalary; }

double Staff::getHourWork() { return hourWork; }