#pragma once

#include "global.h"
#include "core/User.h"

// DTO for next-shift employee info displayed in panel 2
struct ShiftEmployeeInfo {
    QString name;
    QString phone;
    QString role;
    QString avatarPath;
};

// DTO for salary chart data
struct SalaryChartData {
    QVector<double> lastYearMonthly;   // 12 months of (year-1)
    QVector<double> thisYearMonthly;   // 12 months of (year)
    int lastYearEmpCount;
    int thisYearEmpCount;
};

class Dashboard_Model
{
public:
    Dashboard_Model();
    ~Dashboard_Model() = default;

    // Return IDs of employees working the current shift
    QSet<int> getWorkingEmployeeIds();

    // Return employees scheduled for the next shift (name, phone, role, avatar)
    QList<ShiftEmployeeInfo> getNextShiftEmployees();

    // Return salary chart data comparing (year-1) vs (year)
    SalaryChartData getSalaryStats(int year);

private:
};
