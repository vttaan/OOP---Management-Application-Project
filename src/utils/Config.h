#pragma once
#include <global.h>
class Config {
private:
    static const short openHour  = 7, closeHour = 22;
    static const Qt::DayOfWeek dayOpenRegisShift = Qt::Tuesday;
    static const short minStaffPerShift = 4;
    static const short maxStaffPerShift = 6;
public:
    static short getOpenHour(){return openHour;}
    static short getCloseHour(){return closeHour;}
    static Qt::DayOfWeek getDayOpenRegisShift() {return dayOpenRegisShift;}
    static short getMinStaffPerShift() { return minStaffPerShift; }
    static short getMaxStaffPerShift() { return maxStaffPerShift; }
};