#ifndef SALARY_MODEL_H
#define SALARY_MODEL_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include "core/User.h"
#include "utils/Database.h"

using namespace std;

struct SalaryData {
    int normalHours;
    int holidayHours;
    QString normalSalary;
    QString holidaySalary;
    QString penalty;
    QString totalSalary;
};

class Salary_Model : public QObject
{
    Q_OBJECT
public:
    explicit Salary_Model(QObject *parent = nullptr);
    ~Salary_Model();

    // Fetch salary summary for a specific month and year
    SalaryData getSalarySummary(int month, int year);

    // Get table data for normal days and holidays
    // Map key is day, value is hours
    QMap<QString, int> getNormalDaysData(short int id, int month, int year);
    QMap<QString, int> getHolidayDaysData(int month, int year);
    User* currentUser;
    // Base salary formatted string
    QString getBaseSalary();

signals:

};

#endif // SALARY_MODEL_H
