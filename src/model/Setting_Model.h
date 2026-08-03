#ifndef SETTING_MODEL_H
#define SETTING_MODEL_H
#include "global.h"
#include "utils/Database.h"
#include "utils/Config.h"
class Setting_Model
{
public:
    Setting_Model();

    bool loadData(short &openHour, short &closeHour, Qt::DayOfWeek &dayOpenRegis,
                  QMap<QString, QPair<short, short>> &roles,
                  short &maxLeaveFT, short &maxDaysPT, short &maxHourPT);

    bool saveData(short openHour, short closeHour, Qt::DayOfWeek dayOpenRegis,
                  const QMap<QString, QPair<short, short>>& roles,
                  short maxLeaveFT, short maxDaysPT, short maxHourPT);
};

#endif // SETTING_MODEL_H
