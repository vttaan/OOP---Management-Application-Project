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

    if (!view || currentEmployeeId < 0) {
        qDebug() << "Schedule_Control::load() — view or employeeId not set.";
        return;
    }
    // 1. Determine if registration is currently allowed.
    //    Business rule: registration opens on Monday of each week.
    //    Adjust this logic as needed (e.g. read from DB config).
    QDate today = QDate::currentDate();
    bool registrationOpen = (today.dayOfWeek() == Config::getDayOpenRegisShift());
    view->enableRegistration(registrationOpen);
    if(!registrationOpen) return;

    // 2. Set up the input table with days and allowed hours.
    view->setUpDataInputTable(listDays, Config::getOpenHour(), Config::getCloseHour());

    // 3. Fetch this week's shifts from DB into the model.
    model->getSchedule(currentEmployeeId);

    // 4. Build the weekly summary and push it to the view's read-only table.
    view->updateSumTable(buildWeeklySummary());

    qDebug() << "Schedule_Control::load() — loaded schedule for employee" << currentEmployeeId;
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
    model->getSchedule(currentEmployeeId);
    view->updateSumTable(buildWeeklySummary());
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
    // TODO: auto-generate schedule using the selected algorithm
    qDebug() << "Schedule_Control::handleGenSchedule() — not yet implemented.";
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
    // Find the index of this day in listDays (0 = Monday)
    int idx = listDays.indexOf(day);
    if (idx < 0) return QDate(); // invalid

    // Get Monday of the current week
    QDate monday = QDate::currentDate().addDays(-(QDate::currentDate().dayOfWeek() - 1));
    return monday.addDays(idx);
}

QMap<int, QList<QString>> Schedule_Control::buildWeeklySummary() const
{
    // Returns a map: col (0–6) → list of "HH:mm – HH:mm" strings
    QMap<int, QList<QString>> weeklyData;
    const auto& shifts = model->getShiftList();

    for (int col = 0; col < shifts.size(); ++col) {
        QList<QString> labels;
        for (const Shift* s : shifts[col]) {
            QString label = s->getStartTime().toString("HH:mm")
                          + " – "
                          + s->getEndTime().toString("HH:mm");
            labels.append(label);
        }
        if (!labels.isEmpty()) weeklyData.insert(col, labels);
    }
    return weeklyData;
}
