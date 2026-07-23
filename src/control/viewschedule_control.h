#ifndef VIEWSCHEDULE_CONTROL_H
#define VIEWSCHEDULE_CONTROL_H

#include <QObject>
#include "global.h"
#include "model/Schedule_Model.h"
#include "core/ShiftBlock.h"
#include "view/ViewSchedule_View.h"

class SessionManager;

class ViewSchedule_Control : public QObject
{
    Q_OBJECT
private:
    ViewSchedule_View* view;
    Schedule_Model* model;
    short int currentEmployeeId;
    QDate currentViewMonday;

    void updateDateRangeLabel();
    void loadData();
    void loadStaffSchedule();
    void loadManagerSchedule();

public:
    SessionManager* currentSession = nullptr;
    explicit ViewSchedule_Control(QObject *parent = nullptr);
    ~ViewSchedule_Control();

    void setEmployeeId(short int id);
    void load();
    void setView(ViewSchedule_View* view);

private slots:
    void onPrevWeek();
    void onNextWeek();
    void onCurrentWeek();
    void onShiftClicked(int row, int col);
private:
    QMap<int, QMap<int, ShiftBlock*>> scheduleGrid;
};

#endif // VIEWSCHEDULE_CONTROL_H