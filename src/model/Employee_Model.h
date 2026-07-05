#ifndef EMPLOYEE_MODEL_H
#define EMPLOYEE_MODEL_H

#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "core/User.h"
#include "core/UserFactory.h"

class Employee_Model{
public:
    Employee_Model();

    QList<User*> getAllEmployees();

    bool addEmployee(User* emp, const QString& username, const QString& password);

    bool updateEmployee(User* emp);

    bool deleteEmployee(int idEmployee);

private:
    QString saveAvatarLocally(int empId, const QString &sourcePath);
};
#endif // EMPLOYEE_MODEL_H
