#pragma once
#include <QObject>
#include <QMessageBox>
#include <QDebug>
#include <algorithm>
#include "model/Employee_Model.h"
#include "core/UserFactory.h"
#include "view/AddEmployee_Dialog.h"
#include "view/EditEmployee_Dialog.h"


class EmployeesWidget;

class Employee_Control : public QObject {
    Q_OBJECT

public:
    explicit Employee_Control(QObject *parent = nullptr);
    ~Employee_Control();


    void setView(EmployeesWidget *view);
    EmployeesWidget *getView() const;
    QList<User*> searchInEmployee(const QString& contentSearch);
    QList<User*> filterInEmployee(const QStringList& contentFilter);
    QList<User*> sortInEmployee(const QString& typeOrder, const QStringList& contentSort);


    void init();

private slots:

    void handleLoadEmployees();
    void handleAddEmployee();
    void handleEditEmployee(int idEmployee);
    void handleDeleteEmployee(int idEmployee);
    // Combined slot: receives all active view criteria and applies filter→search→sort pipeline
    void handleUpdate(const QString &searchText,
                      const QStringList &roles,
                      const QStringList &genders,
                      const QString &sortField,
                      int sortDir);

private:
    EmployeesWidget  *m_view;
    Employee_Model    m_model;
    QList<User*>      m_employees;
    bool cmpAscending(User* a, User* b, const QStringList& contentSort);
    bool cmpDescending(User* a, User* b, const QStringList& contentSort);
    bool rabinKarp(const QString& pattern, const QString& contentSearch);
};
