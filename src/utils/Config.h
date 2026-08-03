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
    inline static short closeHour;
    inline static Qt::DayOfWeek dayOpenRegisShift;

    static const Qt::DayOfWeek dayOpenRegisShift = Qt::Sunday;
    static const short minStaffPerShift = 4;
    static const short maxStaffPerShift = 6;
    // Number of staff min and max in specific role. First is Min, Second is Max
    inline static QMap<QString, QPair<short, short>> numberEmployeeOfRoles;

    // Rule for full time
    inline static short minximumDaysWorkPerWeek_FT;
    inline static short maximumLeavePerMonth_FT;

    // Rule for part time
    inline static short minximumDaysWorkPerWeek_PT;
    inline static short minximumHourWorkPerDay_PT;
    inline static short maximumHourWorkPerDay_PT;

    inline static short guaranteedDaysPerWeek_FT = 0;

public:
    // ------SETTERS----
    static void setOpenHour(short h) { openHour = h; }
    static void setCloseHour(short h) { closeHour = h; }
    static void setDayOpenRegisShift(Qt::DayOfWeek day) { dayOpenRegisShift = day; }

    static void setRoles(const QMap<QString, QPair<short, short>> &roles)
    {
        numberEmployeeOfRoles = roles;
    }

    static void setMinximumDaysWorkPerWeek_FT(short days) { minximumDaysWorkPerWeek_FT = days; }
    static void setMaximumLeavePerMonth_FT(short days) { maximumLeavePerMonth_FT = days; }

    static void setMinximumDaysWorkPerWeek_PT(short days) { minximumDaysWorkPerWeek_PT = days; }
    static void setMinximumHourWorkPerDay_PT(short hours) { minximumHourWorkPerDay_PT = hours; }
    static void setMaximumHourWorkPerDay_PT(short hours) { maximumHourWorkPerDay_PT = hours; }

    static void setGuaranteedDaysPerWeek_FT(short days) { guaranteedDaysPerWeek_FT = days; }
    // --- GETTERS ---

    static short getOpenHour() { return openHour; }
    static short getCloseHour() { return closeHour; }
    static Qt::DayOfWeek getDayOpenRegisShift() { return dayOpenRegisShift; }

    static QDate getStartOfCurrentWeek(QDate date)
    {
        int diff = dayOpenRegisShift - date.dayOfWeek();
        if (diff > 0)
            diff -= 7;
        return date.addDays(diff);
    }

    static int getMinStaffForRole(const QString &roleName)
    {
        if (numberEmployeeOfRoles.contains(roleName))
            return numberEmployeeOfRoles[roleName].first;
        return 0;
    }

    static int getMaxStaffForRole(const QString &roleName)
    {
        if (numberEmployeeOfRoles.contains(roleName))
            return numberEmployeeOfRoles[roleName].second;
        return 99;
    }

    static QList<QString> getAllRoles()
    {
        return numberEmployeeOfRoles.keys();
    }

    static short getMinximumDaysWorkPerWeek_FT() { return minximumDaysWorkPerWeek_FT; }
    static short getMaximumLeavePerMonth_FT() { return maximumLeavePerMonth_FT; }

    static short getMinximumDaysWorkPerWeek_PT() { return minximumDaysWorkPerWeek_PT; }
    static short getMinximumHourWorkPerDay_PT() { return minximumHourWorkPerDay_PT; }
    static short getMaximumHourWorkPerDay_PT() { return maximumHourWorkPerDay_PT; }
    static short getGuaranteedDaysPerWeek_FT()
    {
        return guaranteedDaysPerWeek_FT > 0 ? guaranteedDaysPerWeek_FT : 5;
    }
};