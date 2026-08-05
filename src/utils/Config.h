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

    // < < < < < < < < < Temporary merge branch 1 static const Qt::DayOfWeek dayOpenRegisShift = Qt::Sunday;
    // static const short minStaffPerShift = 4;
    // static const short maxStaffPerShift = 6;
    // == == == == =
    //static const Qt::DayOfWeek dayOpenRegisShift = Qt::Sunday;
    static const int baseSalaryStaff = 25000;

     // Number of staff min and max in specific role. First is Min, Second is Max
    inline static QMap<QString, QPair<short, short>> numberEmployeeOfRoles;

    // Rule for full time
    inline static short minimumDaysWorkPerWeek_FT;
    inline static short maximumAbsentPerWeek_FT;

    // Rule for part time
    inline static short minimumDaysWorkPerWeek_PT;
    inline static short minimumHourWorkPerDay_PT;
    inline static short maximumHourWorkPerDay_PT;

    inline static short guaranteedDaysPerWeek_FT = 0;

    //inline static double baseSalaryStaff;

public:
    // ------SETTERS----
    static void setOpenHour(short h) { openHour = h; }
    static void setCloseHour(short h) { closeHour = h; }
    static void setDayOpenRegisShift(Qt::DayOfWeek day) { dayOpenRegisShift = day; }

    static void setRoles(const QMap<QString, QPair<short, short>> &roles)
    {
        numberEmployeeOfRoles = roles;
    }

    static void setMinimumDaysWorkPerWeek_FT(short days) { minimumDaysWorkPerWeek_FT = days; }
    static void setMaximumAbsentPerWeek_FT(short days) { maximumAbsentPerWeek_FT = days; }

    static void setMinimumDaysWorkPerWeek_PT(short days) { minimumDaysWorkPerWeek_PT = days; }
    static void setMinimumHourWorkPerDay_PT(short hours) { minimumHourWorkPerDay_PT = hours; }
    static void setMaximumHourWorkPerDay_PT(short hours) { maximumHourWorkPerDay_PT = hours; }

    static void setGuaranteedDaysPerWeek_FT(short days) { guaranteedDaysPerWeek_FT = days; }
    // --- GETTERS ---

    static short getOpenHour() { return openHour; }
    static short getCloseHour() { return closeHour; }
    static Qt::DayOfWeek getDayOpenRegisShift() { return dayOpenRegisShift; }

    static int getMaxStaffPerShift()
    {
        int total = 0;
        for (const QString &role : numberEmployeeOfRoles.keys())
        {
            total += numberEmployeeOfRoles[role].second;
        }
        return total > 0 ? total : 6;
    }

    static int getBaseSalaryStaff() { return baseSalaryStaff; }

    static QDate getStartOfCurrentWeek(QDate date)
    {
        int diff = dayOpenRegisShift - date.dayOfWeek();
        if (diff > 0)
            diff -= 7;
        return date.addDays(diff);
    }

    // The configured change day is a transition day. The newly active
    // schedule begins on the following day and spans seven days.
    static QDate getStartOfActiveWorkingWeek(QDate date)
    {
        if (!date.isValid())
            return {};
        return getStartOfCurrentWeek(date).addDays(1);
    }

    // Maps an arbitrary shift date to the Wednesday-Tuesday style range that
    // contains it. On the transition day itself, that date is the final day
    // of the previous working range.
    static QDate getStartOfWorkingWeekContaining(QDate date)
    {
        if (!date.isValid())
            return {};
        QDate start = getStartOfActiveWorkingWeek(date);
        return start > date ? start.addDays(-7) : start;
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

    static short getMinimumDaysWorkPerWeek_FT() { return minimumDaysWorkPerWeek_FT; }
    static short getMaximumAbsentPerWeek_FT() { return maximumAbsentPerWeek_FT; }
    // SYSTEM_CONFIG stores the full-time monthly leave limit under the
    // historical maximumAbsentPerWeek_FT field used by the settings screen.
    static short getMaximumLeavePerMonth_FT() { return maximumAbsentPerWeek_FT; }

    static short getMinimumDaysWorkPerWeek_PT() { return minimumDaysWorkPerWeek_PT; }
    static short getMinimumHourWorkPerDay_PT() { return minimumHourWorkPerDay_PT; }
    static short getMaximumHourWorkPerDay_PT() { return maximumHourWorkPerDay_PT; }
    static short getGuaranteedDaysPerWeek_FT()
    {
        return guaranteedDaysPerWeek_FT > 0 ? guaranteedDaysPerWeek_FT : 5;
    }
};
