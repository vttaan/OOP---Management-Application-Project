#ifndef SCHEDULE_CONTROL_H
#define SCHEDULE_CONTROL_H

#include "global.h"
#include "model/Schedule_Model.h"
#include "view/Schedule_View.h"
#include "utils/SessionManage.h"
#include "utils/Config.h"
#include "model/LeaveRequest_Model.h"
class Schedule_Control : public QObject
{
    Q_OBJECT
private:
    Schedule_View *view;
    Schedule_Model *model;
    short int currentEmployeeId;

    // Days list provided by business logic (Mon-Sun in the display language)
    QList<QString> listDays;

    // Tracks the week being shown in the assign grid
    QDate currentAssignMonday;
    bool managerWeekInitialized = false;

    // Employee registration mode and the current database-backed full-time grid.
    EmployeeScheduleLayoutMode employeeScheduleLayoutMode =
        EmployeeScheduleLayoutMode::PartTimeHourly;
    FullTimeScheduleGrid fullTimeScheduleStatuses;
    QDate currentEmployeeRegistrationWeekStart;
    QList<ManagerScheduleChange> managerDraftChanges;
    LeaveRequest_Model leaveRequestModel;
    int selectedManagerDay = -1;
    int selectedManagerShift = -1;

    // Helper: convert "Monday" display string -> QDate of that day this week
    QDate dayStringToDate(const QString &day) const;

public:
    explicit Schedule_Control(QObject *parent = nullptr);
    ~Schedule_Control();

    // Called by navigator before showing the page
    void setEmployeeId(short int id);
    short int getEmployeeId() const;
    void setEmployeeScheduleLayoutMode(EmployeeScheduleLayoutMode mode);

    // Core lifecycle
    void load();

    // UML-required stubs (left for future implementation)
    void handleGenSchedule();
    void search();
    void filter();
    void chooseDate();
    void handleChangeAlgorithm();

    // View wiring
    void setView(Schedule_View *view);
    Schedule_View *getView() const;

private slots:
    // Fired by view when the staff presses "Luu / Xac Nhan"
    // selectedHoursByDay: outer index = day (0-6), inner list = selected hour-row indices
    void onSaveGridRequested(const QList<QList<int>> &selectedHoursByDay);

    // Synchronizes editable full-time pending registrations to the database.
    void onSaveFullTimeScheduleRequested(const QList<QList<int>> &selectedShiftsByDay);

    // Fired when manager clicks a shift block in the assign grid
    void onShiftBlockClicked(int col, int row);

    // Fired from the popup dialog approve/decline buttons
    void onApproveShift(PendingShiftInfo request);
    void onDeclineShift(PendingShiftInfo request);
    void onAddEmployeeToShift(int employeeId, QDate date, QTime startTime,
                              QTime endTime, const QString &reason);
    void onRemoveAssignedShift(int shiftId, int employeeId,
                                const QString &reason);

    // Fired when the manager clicks "Xac Nhan"
    void onConfirmRequested();
    void onPreviousManagerWeek();
    void onNextManagerWeek();
    void onCurrentManagerWeek();
    void onUndoManagerDraft();
    void onClearManagerDraft();
    void onLeaveRequested();
    void onLeaveHistoryRequested();


signals:
    void scheduleGenerated(bool success, int assignedCount, const QStringList &warnings);
};

#endif // SCHEDULE_CONTROL_H
