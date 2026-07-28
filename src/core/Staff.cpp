#include "global.h"
#include "core/Staff.h"
#include "model/Salary_Model.h"

Staff::Staff(QString r, short int idEmp, QString ava, QString idCit, QString n,
             QString d, QString add, QString phone, QString gender, int baseSalary)
    : User(r, idEmp, ava, idCit, n, d, add, phone, gender)
    , hourSalary(baseSalary) {}

double Staff::getBaseSalary() const { return hourSalary; }

double Staff::getSalary() const { return Salary_Model::getSalarySummary(this->idEmployee, this->role, this->hourSalary
                                          , QDate::currentDate().month(), QDate::currentDate().year()).totalSalary; }

double Staff::getHourWork() { return hourWork; }