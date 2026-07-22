#ifndef SALARY_CONTROL_H
#define SALARY_CONTROL_H

#include "global.h"
#include <QObject>
#include "model/Salary_Model.h"
#include "view/Salary_View.h"

class Salary_Control : public QObject
{
    Q_OBJECT
private:
    Salary_View* view;
    Salary_Model* model;

public:
    explicit Salary_Control(QObject *parent = nullptr);
    ~Salary_Control();

    void setView(Salary_View* view);
    Salary_View* getView() const;
    
    void loadData(int month, int year);
    void init();

private slots:
    void onMonthYearChanged(int month, int year);
};

#endif // SALARY_CONTROL_H
