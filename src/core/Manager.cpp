#include "global.h"
#include "core/Manager.h"

Manager::Manager(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add, QString phone, QString gender)
    :User(r, idEmp, ava, idCit, n, d, add, phone, gender) {

}

double Manager::getSalary() const {
	return 0.0;
}