#pragma once
#include <global.h>
class Config {
private:
    static const short openHour  = 7, closeHour = 22;
    static const Qt::DayOfWeek dayOpenRegisShift = Qt::Friday;
public:
    static short getOpenHour(){return openHour;}
    static short getCloseHour(){return closeHour;}
    static Qt::DayOfWeek getDayOpenRegisShift() {return dayOpenRegisShift;}
};