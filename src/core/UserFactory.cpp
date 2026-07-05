#include "core/UserFactory.h"
#include <qsqlquery.h>

<<<<<<< Updated upstream

User* UserFactory::createContainsUser(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add,
	QString phone) {
	if (r == "Manager") return new Manager(r, idEmp, ava, idCit, n, d, add, phone);
	else if (r == "Staff") return new Staff(r, idEmp, ava, idCit, n, d, add, phone);
	return nullptr;
}


User* UserFactory::createNewUser(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add, QString phone) {
=======
User *UserFactory::createContainsUser(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add,
                                      QString phone, QString gender)
{
    if (r == "Manager")
        return new Manager(r, idEmp, ava, idCit, n, d, add, phone, gender);
    else if (r == "Staff")
        return new Staff(r, idEmp, ava, idCit, n, d, add, phone, gender);
    return nullptr;
}

User *UserFactory::createNewUser(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add, QString phone,
QString gender)
{
>>>>>>> Stashed changes
    QSqlQuery query;
    short int lastIdOfRole = 0;
    query.prepare("SELECT MAX(IdEmployee) AS MaxID FROM PROFILES WHERE role = : u");
    query.bindValue(":u", r);
<<<<<<< Updated upstream
    if(query.exec()) {
        if(query.next()) lastIdOfRole = query.value("MaxID").toInt();
=======
    if (query.exec())
    {
        if (query.next()) lastIdOfRole = query.value("MaxID").toInt();
>>>>>>> Stashed changes
    }
    else qDebug() << "ERROR IN QUERY\n";

    short int idOfNewUser = lastIdOfRole + 1;
<<<<<<< Updated upstream
    if (r == "Manager") return new Manager(r, idOfNewUser, ava, idCit, n, d, add, phone);
    else if (r == "Staff") return new Staff(r, idOfNewUser, ava, idCit, n, d, add, phone);
=======
    if (r == "Manager")
        return new Manager(r, idOfNewUser, ava, idCit, n, d, add, phone, gender);
    else if (r == "Staff")
        return new Staff(r, idOfNewUser, ava, idCit, n, d, add, phone, gender);
>>>>>>> Stashed changes

    return nullptr;
}