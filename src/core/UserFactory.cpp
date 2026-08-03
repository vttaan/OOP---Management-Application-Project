#include "global.h"
#include "core/UserFactory.h"

User *UserFactory::createContainsUser(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add,
                                      QString phone, QString gender, int baseSalary, bool isFixed, double allowance)
{

    User *user = UserPrototypeRegistry::instance().create(r);
    if (!user) return nullptr;

    user->setIdEmployee(idEmp);
    user->setAva(ava);
    user->setIndentityID(idCit);
    user->setName(n);
    user->setDOB(d);
    user->setAddress(add);
    user->setPhoneNum(phone);
    user->setGender(gender);
    user->setBaseSalary(static_cast<double>(baseSalary));
    user->setFixedEmployee(isFixed);
    user->setAllowenceValue(allowance);

    return user;
}

User *UserFactory::createNewUser(QString r, QString ava, QString idCit, QString n
                                 , QString d, QString add, QString phone, QString gender, int baseSalary, double allowance)
{
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
        qDebug() << "createNewUser: could not fetch MAX id —" << query.lastError().text();
    }

    User *user = UserPrototypeRegistry::instance().create(r);
    if (!user) return nullptr;

    user->setIdEmployee(newId);
    user->setAva(ava);
    user->setIndentityID(idCit);
    user->setName(n);
    user->setDOB(d);
    user->setAddress(add);
    user->setPhoneNum(phone);
    user->setGender(gender);
    user->setBaseSalary(static_cast<double>(baseSalary));
    user->setAllowenceValue(allowance);

    return user;
}