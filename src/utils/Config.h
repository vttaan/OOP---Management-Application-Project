#pragma once
#include <global.h>
class Config
{
private:
    static const short openHour = 7, closeHour = 22;

    static const Qt::DayOfWeek dayOpenRegisShift = Qt::Saturday;
    static const short minStaffPerShift = 4;
    static const short maxStaffPerShift = 6;

public:
    static short getOpenHour() { return openHour; }
    static short getCloseHour() { return closeHour; }
    static Qt::DayOfWeek getDayOpenRegisShift() { return dayOpenRegisShift; }
    static short getMinStaffPerShift() { return minStaffPerShift; }
    static short getMaxStaffPerShift() { return maxStaffPerShift; }

    // Helper to get the start of the current week for a given date
    static QDate getStartOfCurrentWeek(QDate date)
    {
        int diff = dayOpenRegisShift - date.dayOfWeek();
        if (diff > 0)
        {
            diff -= 7;
        }
        return date.addDays(diff);
    }

    // Config for auto schedule
    static const short minDaysPerEmp = 4;
    static short getMinDaysPerEmp() { return minDaysPerEmp; }
};