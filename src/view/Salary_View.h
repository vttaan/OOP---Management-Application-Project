#ifndef SALARY_VIEW_H
#define SALARY_VIEW_H

#include "global.h"
#include <QWidget>
#include <QMap>
#include "model/Salary_Model.h"

namespace Ui {
class Salary_View;
}

class Salary_View : public QWidget
{
    Q_OBJECT

public:
    explicit Salary_View(QWidget *parent = nullptr);
    ~Salary_View();

    void setBaseSalary(const QString& salary);
    void populateNormalTable(const QMap<QString, int>& data);
    void populateHolidayTable(const QMap<QString, int>& data);
    void populateSummaryTable(const SalaryData& data);

signals:
    void monthYearChanged(int month, int year);

private:
    Ui::Salary_View *ui;
    void setupUI();
    void setupConnections();
};

#endif // SALARY_VIEW_H
