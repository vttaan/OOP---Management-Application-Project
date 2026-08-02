#include "global.h"
#include "core/UserFactory.h"

User *UserFactory::createContainsUser(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add,
                                      QString phone, QString gender, int baseSalary, bool isFixed)
{
    if (r == "Manager")
        return new Manager(r, idEmp, ava, idCit, n, d, add, phone, gender, baseSalary);
    else if (r == "Staff")
        return new Staff(r, idEmp, ava, idCit, n, d, add, phone, gender, baseSalary);
    else if (r == "KitchenAssistant")
        return new KitchenAssistant(idEmp, ava, idCit, n, d, add, phone, gender, baseSalary);
    else if (r == "HallStaff")
        return new HallStaff(idEmp, ava, idCit, n, d, add, phone, gender, baseSalary);
    else if (r == "Cashier") {
        if (isFixed)
            return new Cashier(idEmp, ava, idCit, n, d, add, phone, gender, 0, (double)baseSalary);
        else
            return new Cashier(idEmp, ava, idCit, n, d, add, phone, gender, baseSalary);
    }
    return nullptr;
}

User *UserFactory::createNewUser(QString r, QString ava, QString idCit, QString n
                                 , QString d, QString add, QString phone, QString gender, int baseSalary)
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

    if (r == "Manager")
        return new Manager(r, newId, ava, idCit, n, d, add, phone, gender, baseSalary);
    else if (r == "Staff")
        return new Staff(r, newId, ava, idCit, n, d, add, phone, gender, baseSalary);
    else if (r == "KitchenAssistant")
        return new KitchenAssistant(newId, ava, idCit, n, d, add, phone, gender, baseSalary);
    else if (r == "HallStaff")
        return new HallStaff(newId, ava, idCit, n, d, add, phone, gender, baseSalary);
    else if (r == "Cashier")
        return new Cashier(newId, ava, idCit, n, d, add, phone, gender, baseSalary);
    return nullptr;
}