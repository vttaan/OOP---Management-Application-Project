#pragma once
#include "core/User.h"
#include <vector>
#include "utils/Database.h"
#include "core/UserFactory.h"
#include <algorithm>
using namespace std;

class Employee_Model
{
private:
    vector<User*> listEmployee;
    QString getPattern(short i);
    bool cmpAscending(User* a, User* b, vector<QString> contentSort);
    bool cmpDescending(User* a, User* b, vector<QString> contentSort);
    bool rabinKarp(QString pattern, QString contentSearch);
public:
    Employee_Model();
    vector<User*> getListEmployee(){return this->listEmployee;}
    void addUserInListEmployee(User* a);
    void deleteUserInListEmployee(User* a);
    void loadData();
    vector<User*> searchInEmployee(QString contentSearch);
    vector<User*> filterInEmployee(vector<QString> contentFilter);
    vector<User*> sortInEmployee(QString typeOrder, vector<QString> contentSort);
};

