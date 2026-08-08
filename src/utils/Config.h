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
    static QString canonicalRoleName(const QString &roleName)
    {
        const QString role = roleName.trimmed();
        if (role.compare("Cashier", Qt::CaseInsensitive) == 0)
            return "Cashier";
        if (role.compare("HallStaff", Qt::CaseInsensitive) == 0)
            return "HallStaff";
        if (role.compare("KitchenAssistant", Qt::CaseInsensitive) == 0 ||
            role.compare("KitchenAssitant", Qt::CaseInsensitive) == 0)
            return "KitchenAssistant";
        if (role.compare("Manager", Qt::CaseInsensitive) == 0 ||
            role.compare("Manage", Qt::CaseInsensitive) == 0)
            return "Manager";
        if (role.compare("Admin", Qt::CaseInsensitive) == 0)
            return "Admin";
        return role;
    }

    static QString displayRoleName(const QString &roleName)
    {
        const QString role = canonicalRoleName(roleName);
        if (role.compare("Cashier", Qt::CaseInsensitive) == 0)
            return QString::fromUtf8("Thu ngân");
        if (role.compare("HallStaff", Qt::CaseInsensitive) == 0)
            return QString::fromUtf8("Nhân viên sảnh");
        if (role.compare("KitchenAssistant", Qt::CaseInsensitive) == 0)
            return QString::fromUtf8("Phụ bếp");
        if (role.compare("Manager", Qt::CaseInsensitive) == 0)
            return QString::fromUtf8("Quản lý");
        if (role.compare("Admin", Qt::CaseInsensitive) == 0)
            return QString::fromUtf8("Quản trị viên");
        return roleName;
    }
    static QTime getShiftStartTime(int shiftIndex)
    {
        switch (shiftIndex)
        {
        case 0: return QTime(openHour, 0);
        case 1: return QTime(12, 0);
        case 2: return QTime(17, 0);
        default: return {};
        }
    }

    static QTime getShiftEndTime(int shiftIndex)
    {
        switch (shiftIndex)
        {
        case 0: return QTime(12, 0);
        case 1: return QTime(17, 0);
        case 2: return QTime(closeHour, 0);
        default: return {};
        }
    }

    static QString getShiftTimeLabel(int shiftIndex)
    {
        const QTime start = getShiftStartTime(shiftIndex);
        const QTime end = getShiftEndTime(shiftIndex);
        if (!start.isValid() || !end.isValid())
            return {};
        return QString("%1 - %2").arg(start.toString("HH:mm"),
                                      end.toString("HH:mm"));
    }
    static Qt::DayOfWeek getDayOpenRegisShift() { return dayOpenRegisShift; }

    static int getMaxStaffPerShift()
    {
        int total = 0;
        for (const QString &role : getOperationalRoles())
        {
            total += getMaxStaffForRole(role);
        }
        return total > 0 ? total : 6;
    }

    static int getBaseSalaryStaff() { return baseSalaryStaff; }

    static QDate getStartOfCurrentWeek(QDate date)
    {
        if (!date.isValid())
            return {};
        return date.addDays(Qt::Monday - date.dayOfWeek());
    }

    static QDate getStartOfNextWeek(QDate date)
    {
        return getStartOfCurrentWeek(date).addDays(7);
    }

    static int getMinStaffForRole(const QString &roleName)
    {
        const QString canonicalRole = canonicalRoleName(roleName);
        for (auto it = numberEmployeeOfRoles.constBegin();
             it != numberEmployeeOfRoles.constEnd(); ++it)
            if (canonicalRoleName(it.key()).compare(
                    canonicalRole, Qt::CaseInsensitive) == 0)
                return it.value().first;
        return 0;
    }

    static int getMaxStaffForRole(const QString &roleName)
    {
        const QString canonicalRole = canonicalRoleName(roleName);
        for (auto it = numberEmployeeOfRoles.constBegin();
             it != numberEmployeeOfRoles.constEnd(); ++it)
            if (canonicalRoleName(it.key()).compare(
                    canonicalRole, Qt::CaseInsensitive) == 0)
                return it.value().second;
        return 99;
    }

    static QList<QString> getAllRoles()
    {
        return numberEmployeeOfRoles.keys();
    }

    static bool isOperationalRole(const QString &roleName)
    {
        const QString role = canonicalRoleName(roleName);
        return !role.isEmpty() &&
               role.compare("Manager", Qt::CaseInsensitive) != 0 &&
               role.compare("Manage", Qt::CaseInsensitive) != 0 &&
               role.compare("Admin", Qt::CaseInsensitive) != 0;
    }

    static QList<QString> getOperationalRoles()
    {
        QList<QString> roles;
        for (const QString &role : numberEmployeeOfRoles.keys())
            if (isOperationalRole(role))
            {
                const QString canonicalRole = canonicalRoleName(role);
                if (!roles.contains(canonicalRole))
                    roles.append(canonicalRole);
            }
        return roles;
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
