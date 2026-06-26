#include "core/UserFactory.h"


User* UserFactory::createNewUser(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add,
	QString phone) {
	if (r == "Manager") return new Manager(r, idEmp, ava, idCit, n, d, add, phone);
	else if (r == "Staff") return new Staff(r, idEmp, ava, idCit, n, d, add, phone);
	return nullptr;
}