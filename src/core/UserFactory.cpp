#include "global.h"
#include "core/UserFactory.h"

User *UserFactory::createContainsUser(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add,
                                      QString phone, QString gender)
{
    if (r == "Manage")
        return new Manager(r, idEmp, ava, idCit, n, d, add, phone, gender);
    else if (r == "Staff")
        return new Staff(r, idEmp, ava, idCit, n, d, add, phone, gender);
    return nullptr;
}

User *UserFactory::createNewUser(QString r, QString ava, QString idCit, QString n, QString d, QString add, QString phone, QString gender)
{
    // Get the MAX idEmployee in the table to create a unique new ID
    QSqlQuery query;
    short int newId = 1;
    query.prepare("SELECT MAX(idEmployee) AS MaxID FROM PROFILES WHERE role = :u");
    query.bindValue(":u", r);
    if (query.exec() && query.next())
    {
        QVariant v = query.value("MaxID");
        if (!v.isNull())
            newId = static_cast<short int>(v.toInt() + 1);
    }
    else
    {
        // qDebug() << "createNewUser: could not fetch MAX id —" << query.lastError().text();
    }

    if (r == "Manage")
        return new Manager(r, newId, ava, idCit, n, d, add, phone, gender);
    else if (r == "Staff")
        return new Staff(r, newId, ava, idCit, n, d, add, phone, gender);

    return nullptr;
}