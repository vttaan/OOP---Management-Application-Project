#ifndef SETTING_CONTROL_H
#define SETTING_CONTROL_H

#include "global.h"
#include "view/Setting_View.h"
#include "model/Setting_Model.h"

class Setting_Control : public QObject
{
    Q_OBJECT
public:

    explicit Setting_Control(QObject *parent = nullptr);
    ~Setting_Control();

    void setView(Setting_View* v);
    void init();

private slots:

    void handleSave(short openHour, short closeHour, Qt::DayOfWeek dayOpenRegis,
                    QMap<QString, QPair<short, short>> roles, short maxLeaveFT,
                    short maxDaysPT, short maxHourPT);


    void handleCancel();

private:
    Setting_View *view;
    Setting_Model *model;
};

#endif