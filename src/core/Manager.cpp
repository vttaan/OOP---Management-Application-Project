#include "global.h"
#include "core/Manager.h"

Manager::Manager(QString r, short int idEmp, QString ava, QString idCit, QString n
                 , QString d, QString add, QString phone, QString gender, int baseSalary)
    :User(r, idEmp, ava, idCit, n, d, add, phone, gender)
    , fixSalary(baseSalary){

}

double Manager::getSalary() const {
    return fixSalary;
}