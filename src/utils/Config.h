#pragma once
#include <global.h>
#include <QDate>
#include <QMap>
#include <QString>
#include <QList>
#include <QPair>
#include <QSqlQuery>
#include <QSqlError>

class Config
{
private:
    // Time
    inline static short openHour;
    inline static short closeHour ;
    inline static Qt::DayOfWeek dayOpenRegisShift;

    //static const Qt::DayOfWeek dayOpenRegisShift = Qt::Sunday;
    static const int baseSalaryStaff = 25000;
    // Number of staff min and max in specific role. First is Min, Second is Max
    inline static QMap<QString, QPair<short, short>> numberEmployeeOfRoles;

    // Rule for full time
    inline static short minimumDaysWorkPerWeek_FT;
    inline static short maximumLeavePerMonth_FT;

    // Rule for part time
    inline static short minimumDaysWorkPerWeek_PT;
    inline static short minimumHourWorkPerDay_PT;
    inline static short maximumHourWorkPerDay_PT;

    inline static short guaranteedDaysPerWeek_FT = 0;

public:
    // ------SETTERS----
    static void setOpenHour(short h) { openHour = h; }
    static void setCloseHour(short h) { closeHour = h; }
    static void setDayOpenRegisShift(Qt::DayOfWeek day) { dayOpenRegisShift = day; }

    static void setRoles(const QMap<QString, QPair<short, short>>& roles) {
        numberEmployeeOfRoles = roles;
    }

    static void setMinimumDaysWorkPerWeek_FT(short days) { minimumDaysWorkPerWeek_FT = days; }
    static void setMaximumLeavePerMonth_FT(short days) { maximumLeavePerMonth_FT = days; }

    static void setMinimumDaysWorkPerWeek_PT(short days) { minimumDaysWorkPerWeek_PT = days; }
    static void setMinimumHourWorkPerDay_PT(short hours) { minimumHourWorkPerDay_PT = hours; }
    static void setMaximumHourWorkPerDay_PT(short hours) { maximumHourWorkPerDay_PT = hours; }

    static void setGuaranteedDaysPerWeek_FT(short days) { guaranteedDaysPerWeek_FT = days; }
    // --- GETTERS ---

    static short getOpenHour() { return openHour; }
    static short getCloseHour() { return closeHour; }
    static Qt::DayOfWeek getDayOpenRegisShift() { return dayOpenRegisShift; }

    static int getMaxStaffPerShift() {
        int total = 0;
        for (const QString& role : numberEmployeeOfRoles.keys()) {
            total += numberEmployeeOfRoles[role].second;
        }
        return total > 0 ? total : 6;
    }

    static int getBaseSalaryStaff() { return baseSalaryStaff; }



    static QDate getStartOfCurrentWeek(QDate date)
    {
        int diff = dayOpenRegisShift - date.dayOfWeek();
        if (diff > 0) diff -= 7;
        return date.addDays(diff);
    }

    static int getMinStaffForRole(const QString& roleName) {
        if (numberEmployeeOfRoles.contains(roleName))
            return numberEmployeeOfRoles[roleName].first;
        return 0;
    }

    static int getMaxStaffForRole(const QString& roleName) {
        if (numberEmployeeOfRoles.contains(roleName))
            return numberEmployeeOfRoles[roleName].second;
        return 99;
    }

    static QList<QString> getAllRoles() {
        return numberEmployeeOfRoles.keys();
    }

    static short getMinimumDaysWorkPerWeek_FT() {  return minimumDaysWorkPerWeek_FT; }
    static short getMaximumLeavePerMonth_FT() { return maximumLeavePerMonth_FT; }

    static short getMinimumDaysWorkPerWeek_PT() { return minimumDaysWorkPerWeek_PT ; }
    static short getMinimumHourWorkPerDay_PT() { return minimumHourWorkPerDay_PT ; }
    static short getMaximumHourWorkPerDay_PT() { return maximumHourWorkPerDay_PT ; }
    static short getGuaranteedDaysPerWeek_FT() {
        return guaranteedDaysPerWeek_FT > 0 ? guaranteedDaysPerWeek_FT : 5;
    }
};