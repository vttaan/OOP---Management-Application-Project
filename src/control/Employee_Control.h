#pragma once

#include "global.h"
#include "model/Employee_Model.h"
#include "core/UserFactory.h"
#include "view/AddEmployee_Dialog.h"
#include "view/EditEmployee_Dialog.h"

class Employee_View;

class Employee_Control : public QObject {
    Q_OBJECT

public:
    explicit Employee_Control(QObject *parent = nullptr);
    //Employee_Control();
    ~Employee_Control();

    void setView(Employee_View *view);
    void setModel(Employee_Model* emp);
    Employee_View *getView() const;
    void init();
signals:
    void profilePageClicked();
    void backToDashBoard();
private slots:

    void handleLoadEmployees();
    void handleAddEmployee();
    void handleEditEmployee(int idEmployee);
    void handleDeleteEmployee(int idEmployee);

    void handleUpdate(const QString &searchText,
                      const QList<QString> &contentFilter,
                      const QList<QString> &contentSort,
                      short sortDir);

private:
    Employee_View  *m_view;
    Employee_Model   *m_model;
};
