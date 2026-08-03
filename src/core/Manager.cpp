#include "global.h"
#include "core/Manager.h"
#include "model/Salary_Model.h"
#include "userprototyperegistry.h"
Manager::Manager(QString r, short int idEmp, QString ava, QString idCit, QString n
                 , QString d, QString add, QString phone, QString gender, int baseSalary)
    :User(r, idEmp, ava, idCit, n, d, add, phone, gender)
    , fixSalary(baseSalary){

}

double Manager::getBaseSalary() const {
    return fixSalary;
}

double Manager::getSalary() const {
    return Salary_Model::getSalarySummary(this->idEmployee, this->role, this->fixSalary
                                          , QDate::currentDate().month(), QDate::currentDate().year()).totalSalary;
}

User* Manager::clone() const {
    return new Manager(*this);
}

namespace {
struct ManagerRegistrar {
    ManagerRegistrar() {
        UserPrototypeRegistry::instance().registerPrototype(
            "Manager",
            new Manager("Manager", 0, "", "", "", "", "", "", "", 0));
    }
} managerRegistrar;
}