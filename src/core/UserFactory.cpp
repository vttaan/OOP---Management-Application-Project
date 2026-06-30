#include "core/UserFactory.h"
#include <qsqlquery.h>


User* UserFactory::createContainsUser(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add,
	QString phone) {
	if (r == "Manager") return new Manager(r, idEmp, ava, idCit, n, d, add, phone);
	else if (r == "Staff") return new Staff(r, idEmp, ava, idCit, n, d, add, phone);
	return nullptr;
}


User* UserFactory::createNewUser(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add, QString phone) {
    QSqlQuery query;
    short int lastIdOfRole = 0;
    query.prepare("SECLECT MAX(IdEmployee) AS MaxID FROM PROFILES WHERE role = : u");
    query.bindValue(":u", r);
    if(query.exec()) {
        if(query.next()) lastIdOfRole = query.value("MaxID").toInt();
    }
    else qDebug() << "ERROR IN QUERY\n";

    short int idOfNewUser = lastIdOfRole + 1;
    if (r == "Manager") return new Manager(r, idOfNewUser, ava, idCit, n, d, add, phone);
    else if (r == "Staff") return new Staff(r, idOfNewUser, ava, idCit, n, d, add, phone);

    return nullptr;
}