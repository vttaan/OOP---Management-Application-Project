#ifndef SCHEDULE_CONTROL_H
#define SCHEDULE_CONTROL_H

#include "global.h"
#include "model/Schedule_Model.h"
#include "view/Schedule_View.h"
#include "utils/SessionManage.h"
#include "utils/Config.h"
class Schedule_Control : public QObject
{
    Q_OBJECT
private:
    Schedule_View*   view;
    Schedule_Model*  model;
    short int        currentEmployeeId;

    // Days list provided by business logic (Mon–Sun in the display language)
    QList<QString>   listDays;


    // Helper: convert "Monday" display string → QDate of that day this week
    QDate dayStringToDate(const QString& day) const;

    // Build the QMap<col, QList<label>> the view needs for the summary table
    QMap<int, QList<QString>> buildWeeklySummary() const;

public:
    explicit Schedule_Control(QObject *parent = nullptr);
    ~Schedule_Control();

    // Called by navigator before showing the page
    void setEmployeeId(short int id);
    short int getEmployeeId() const;

    // Core lifecycle
    void load();

    // UML-required stubs (left for future implementation)
    void handleSaveSchedule();
    void handleGenSchedule();
    void search();
    void filter();
    void chooseDate();
    void handleChangeAlgorithm();

    // View wiring
    void setView(Schedule_View* view);
    Schedule_View* getView() const;

private slots:
    // Fired by view when the user presses "Thêm" (Add shift)
    void onAddShiftRequested(const QString& day,
                             const QString& startTime,
                             const QString& endTime);

    // Fired by view when the user presses "Lưu" (Save)
    void onSaveShiftRequested();
};

#endif // SCHEDULE_CONTROL_H
