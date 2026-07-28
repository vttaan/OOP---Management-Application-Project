#ifndef SALARY_MODEL_H
#define SALARY_MODEL_H

#include "global.h"
#include "core/User.h"
#include "core/Staff.h"
#include "utils/Database.h"
#include "core/Shift.h"

using namespace std;

struct SalaryData {
    int normalHours;
    int holidayHours;
    int normalSalary;
    int holidaySalary;
    int penalty;
    int totalSalary;
};

class Salary_Model : public QObject
{
    Q_OBJECT
public:
    explicit Salary_Model(QObject *parent = nullptr);
    ~Salary_Model();


    static SalaryData getSalarySummary(short int id, QString role, double base, int month, int year);

    static QMap<QString, int> getNormalDaysData(short int id, QString role, double base, int month, int year);
    static QMap<QString, int> getHolidayDaysData(short int id, QString role, double base, int month, int year);
    User* currentUser;


signals:

};

#endif // SALARY_MODEL_H
