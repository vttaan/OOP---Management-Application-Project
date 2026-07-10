#ifndef SCHEDULE_CONTROL_H
#define SCHEDULE_CONTROL_H

#include "global.h"
#include "model/Schedule_Model.h"

// Forward declare the view so the team can implement it later
class Schedule_View;

class Schedule_Control : public QObject
{
    Q_OBJECT
private:
    Schedule_View* view;
    Schedule_Model* model;

public:
    explicit Schedule_Control(QObject *parent = nullptr);
    ~Schedule_Control();

    // Core functionality as per UML
    void load();
    void handleSaveSchedule();
    void handleGenSchedule();
    void search();
    void filter();
    void chooseDate();
    void handleChangeAlgorithm();
    
    // Setters/Getters
    void setView(Schedule_View* view);
    Schedule_View* getView() const;
};

#endif // SCHEDULE_CONTROL_H
