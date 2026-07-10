#pragma once
#include "global.h"
#include "core/User.h"
#include "utils/Database.h"
#include "core/UserFactory.h"

using namespace std;

class Employee_Model
{
private:
    QList<User *> listEmployee;
    QString getPattern(User *emp);
    bool cmpAscending(User *a, User *b, QList<QString> contentSort);
    bool cmpDescending(User *a, User *b, QList<QString> contentSort);
    bool rabinKarp(QString pattern, QString contentSearch);
    QString saveAvatarLocally(int empId, const QString &sourcePath);

    //--------REMOVE ACCENT FOR SEARCH
    QString removeAccent(const QString &input);
    //-----------------------------------
    QList<User *> searchInEmployee(QList<User *> inputList, QString contentSearch);
    QList<User *> filterInEmployee(QList<User *> inputList, QList<QString> contentFilter);
    QList<User *> sortInEmployee(QList<User *> inputList, short typeOrder, QList<QString> contentSort);

public:
    Employee_Model();
    ~Employee_Model()
    {
        qDeleteAll(listEmployee);
        listEmployee.clear();
    }
    QList<User *> getListEmployee() { return this->listEmployee; }
    bool addUserInList(User *emp);
    bool popUserInList(short idEmployee);
    void loadData();

    QList<User *> SearchSortFilter(QString contentSearch, short typeOrder, QList<QString> contentSort,
                                   QList<QString> contentFilter);

    bool addEmployee(const QString &role, const QString &avatarPath, const QString &citizenId,
                     const QString &name, const QString &dob, const QString &address,
                     const QString &phone, const QString &gender,
                     const QString &username, const QString &password);

    bool updateEmployee(User *emp);
    bool deleteEmployee(short idEmployee);
};
