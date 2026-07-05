#include "core/Staff.h"

Staff::Staff(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add, QString phone)
	: User(r, idEmp, ava, idCit, n, d, add, phone)
{
}

double Staff::getSalary() const
{
	return 0.0;
}
