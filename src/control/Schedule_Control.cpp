#include "global.h"
#include "control/Schedule_Control.h"

// ─────────────────────────────────────────────
// Construction / Destruction
// ─────────────────────────────────────────────

Schedule_Control::Schedule_Control(QObject *parent)
    : QObject(parent)
    , view(nullptr)
    , model(new Schedule_Model())
    , currentEmployeeId(-1)
{
    // Default day labels (Vietnamese, Monday first)
    listDays = {"Thứ 2", "Thứ 3", "Thứ 4", "Thứ 5", "Thứ 6", "Thứ 7", "CN"};
}

Schedule_Control::~Schedule_Control()
{
    delete model;
    // view is managed by the navigator / parent widget — do not delete here
}

// ─────────────────────────────────────────────
// Employee ID
// ─────────────────────────────────────────────

void Schedule_Control::setEmployeeId(short int id)
{
    currentEmployeeId = id;
}

short int Schedule_Control::getEmployeeId() const
{
    return currentEmployeeId;
}

// ─────────────────────────────────────────────
// View wiring
// ─────────────────────────────────────────────

void Schedule_Control::setView(Schedule_View* v)
{
    view = v;
    if (!view) return;

    // Connect view signals → controller slots
    connect(view, &Schedule_View::requestAddShift,
            this, &Schedule_Control::onAddShiftRequested);

    connect(view, &Schedule_View::requestSaveShift,
            this, &Schedule_Control::onSaveShiftRequested);

    connect(view, &Schedule_View::requestGenSchedule,
            this, &Schedule_Control::handleGenSchedule);
}

Schedule_View* Schedule_Control::getView() const
{
    return view;
}

// ─────────────────────────────────────────────
// Core lifecycle: load()
// ─────────────────────────────────────────────

void Schedule_Control::load()
{
    if (!view) return;
    
    bool isManager = SessionManager::getInstance()->checkPermission("Manager");
    view->setManagerMode(isManager);

    if (isManager) {
        QDate today = QDate::currentDate();
        // Quản lý sẽ xem và xếp lịch cho TUẦN SAU
        QDate monday = today.addDays(8 - today.dayOfWeek());
        
        QMap<int, QMap<int, ShiftBlock*>> grid = model->getManagerWeeklyGrid(monday, 0);
        view->updateManagerPendingGrid(grid);
        view->updateTableHeaders(monday);
        
    } else {
        if (currentEmployeeId < 0) return;
        
        QDate today = QDate::currentDate();
        bool registrationOpen = (today.dayOfWeek() == Config::getDayOpenRegisShift());
        
        view->enableRegistration(registrationOpen);
        if (!registrationOpen) return;
        
        QDate monday = today.addDays(8 - today.dayOfWeek());
        view->setUpDataInputTable(monday, Config::getOpenHour(), Config::getCloseHour());
        
        // Employee view table shows the NEXT week to match registration
        view->updateTableHeaders(monday);
        model->getSchedule(currentEmployeeId, monday);
        view->updateSumTable(model->getWeeklySummaryStrings());
    }
}

// ─────────────────────────────────────────────
// Slot: user pressed "Thêm" (Add)
// ─────────────────────────────────────────────

void Schedule_Control::onAddShiftRequested(const QString& day,
                                           const QString& startTime,
                                           const QString& endTime)
{
    // Parse selected date
    QDate shiftDate = dayStringToDate(day);
    if (!shiftDate.isValid()) {
        qDebug() << "Schedule_Control: invalid day string" << day;
        return;
    }

    // Parse times ("7:00", "13:00", etc.)
    QTime start = QTime::fromString(startTime, "H:mm");
    QTime end   = QTime::fromString(endTime,   "H:mm");

    if (!start.isValid() || !end.isValid()) {
        view->showError("Thời gian không hợp lệ.");
        return;
    }

    if (start >= end) {
        view->showError("Giờ bắt đầu phải nhỏ hơn giờ kết thúc.");
        return;
    }

    // Attempt to add via model (overlap check + DB insert)
    Shift* newShift = model->handleAddShiftSubmission(currentEmployeeId, shiftDate, start, end);

    if (!newShift) {
        // Model returned nullptr → overlap or DB error
        view->showError("Ca làm bị trùng giờ hoặc không thể thêm vào cơ sở dữ liệu.");
        return;
    }

    // Success: reload the in-memory list from DB and refresh summary table
    // Fetch schedule for the NEXT week (the week that was just registered for)
    QDate monday = QDate::currentDate().addDays(8 - QDate::currentDate().dayOfWeek());
    model->getSchedule(currentEmployeeId, monday);
    view->updateSumTable(model->getWeeklySummaryStrings());
    view->resetInputTable();

    delete newShift; // Ownership stays in the model's shiftList after reload

    qDebug() << "Schedule_Control: shift added —" << day << startTime << "→" << endTime;
}

// ─────────────────────────────────────────────
// Slot: user pressed "Lưu" (Save)
// ─────────────────────────────────────────────

void Schedule_Control::onSaveShiftRequested()
{
    // Currently the shift is persisted immediately in handleAddShiftSubmission.
    // "Save" can be used to trigger a confirmation dialog or a final batch-commit.
    handleSaveSchedule();
}

// ─────────────────────────────────────────────
// UML stubs — implement in future sprints
// ─────────────────────────────────────────────

void Schedule_Control::handleSaveSchedule()
{
    qDebug() << "Schedule_Control::handleSaveSchedule() — all pending shifts are already persisted.";
    // TODO: if we move to a draft/commit model, flush pending shifts here.
}

void Schedule_Control::handleGenSchedule()
{
    if (!model || !view) return;
    QStringList warnings = model->generateSchedule();
    
    view->showSuccess("Xếp lịch tự động hoàn tất!");
    if (!warnings.isEmpty()) {
        view->showError(warnings.join("\n"));
    }
    
    // Refresh the grid
    load();
}

void Schedule_Control::search()
{
    qDebug() << "Schedule_Control::search() — not yet implemented.";
}

void Schedule_Control::filter()
{
    qDebug() << "Schedule_Control::filter() — not yet implemented.";
}

void Schedule_Control::chooseDate()
{
    qDebug() << "Schedule_Control::chooseDate() — not yet implemented.";
}

void Schedule_Control::handleChangeAlgorithm()
{
    qDebug() << "Schedule_Control::handleChangeAlgorithm() — not yet implemented.";
}

// ─────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────

QDate Schedule_Control::dayStringToDate(const QString& day) const
{
    // Find the index of this day by matching prefix with listDays (0 = Monday)
    int idx = -1;
    for (int i = 0; i < listDays.size(); ++i) {
        if (day.startsWith(listDays[i])) {
            idx = i;
            break;
        }
    }
    if (idx < 0) return QDate(); // invalid

    // Nhân viên đăng ký lịch làm là cho TUẦN SAU
    QDate monday = QDate::currentDate().addDays(8 - QDate::currentDate().dayOfWeek());
    return monday.addDays(idx);
}
