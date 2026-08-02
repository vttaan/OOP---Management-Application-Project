#include "global.h"
#include "control/Schedule_Control.h"
#include "view/Schedule_View.h"

// ─────────────────────────────────────────────
// Construction / Destruction
// ─────────────────────────────────────────────

Schedule_Control::Schedule_Control(QObject *parent)
    : QObject(parent), view(nullptr), model(new Schedule_Model()), currentEmployeeId(-1)
{
    // Default day labels (Vietnamese, Monday first)
    listDays = {"Thứ 2", "Thứ 3", "Thứ 4", "Thứ 5", "Thứ 6", "Thứ 7", "CN"};
    initializeFullTimeMockStatuses();
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

void Schedule_Control::setEmployeeScheduleLayoutMode(EmployeeScheduleLayoutMode mode)
{
    employeeScheduleLayoutMode = mode;
}

void Schedule_Control::initializeFullTimeMockStatuses()
{
    using Status = FullTimeShiftStatus;
    fullTimeMockStatuses = {
        {Status::Registered,   Status::Unregistered, Status::StaffShortage},
        {Status::Unregistered, Status::Registered,   Status::Unregistered},
        {Status::StaffShortage, Status::Unregistered, Status::Registered},
        {Status::Unregistered, Status::Unregistered, Status::Unregistered},
        {Status::Registered,   Status::StaffShortage, Status::Unregistered},
        {Status::Unregistered, Status::Registered,   Status::Unregistered},
        {Status::StaffShortage, Status::Unregistered, Status::Registered}
    };
}

// ─────────────────────────────────────────────
// View wiring
// ─────────────────────────────────────────────

void Schedule_Control::setView(Schedule_View *v)
{
    view = v;
    if (!view)
        return;

    // Connect view signals -> controller slots
    connect(view, &Schedule_View::requestSaveGridShifts,
            this, &Schedule_Control::onSaveGridRequested);

    connect(view, &Schedule_View::requestSaveFullTimeShifts,
            this, &Schedule_Control::onSaveFullTimeShiftsRequested);

    connect(view, &Schedule_View::requestGenSchedule,
            this, &Schedule_Control::handleGenSchedule);

    connect(view, &Schedule_View::requestConfirm,
            this, &Schedule_Control::onConfirmRequested);

    connect(view, &Schedule_View::shiftBlockClicked,
            this, &Schedule_Control::onShiftBlockClicked);

    connect(view, &Schedule_View::requestApproveShift,
            this, &Schedule_Control::onApproveShift);

    connect(view, &Schedule_View::requestDeclineShift,
            this, &Schedule_Control::onDeclineShift);
}

Schedule_View *Schedule_Control::getView() const
{
    return view;
}

// ─────────────────────────────────────────────
// Core lifecycle: load()
// ─────────────────────────────────────────────

void Schedule_Control::load()
{
    if (!view)
        return;

    bool isManager = SessionManager::getInstance()->checkPermission("Manager");
    view->setManagerMode(isManager);

    if (isManager)
    {
        QDate today = QDate::currentDate();
        // Manager sees and assigns schedule for NEXT WEEK
        currentAssignMonday = Config::getStartOfCurrentWeek(today).addDays(7);

        // Update column headers with dates
        view->updateTableHeaders(currentAssignMonday);

        // ── Xep Lich Lam grid: show all statuses ──
        QMap<int, QMap<int, BlockCounts>> counts =
            model->getAssignBlockCounts(currentAssignMonday);
        view->updateAssignGrid(counts);

        // ── Build missing-shift info for the bottom table ──
        // For the missing-shift table we use the accepted-only grid
        QMap<int, QMap<int, ShiftBlock *>> acceptedGrid =
            model->getManagerWeeklyGrid(currentAssignMonday, 1);

        const QStringList shiftNames = {"Ca Sáng", "Ca Chiều", "Ca Tối"};
        const QStringList dayNames = {"Thứ 2", "Thứ 3", "Thứ 4", "Thứ 5", "Thứ 6", "Thứ 7", "CN"};
        QList<MissingShiftInfo> missingList;
        for (int col = 0; col < 7; ++col)
        {
            if (!acceptedGrid.contains(col))
                continue;
            for (int row = 0; row < 3; ++row)
            {
                if (!acceptedGrid[col].contains(row))
                    continue;
                ShiftBlock *block = acceptedGrid[col][row];
                if (!block)
                    continue;
                if (block->getStatus() == ShiftStatus::Sufficient)
                    continue;
                MissingShiftInfo info;
                info.dateStr = currentAssignMonday.addDays(col).toString("dd/MM/yyyy");
                info.shiftName = (row < shiftNames.size()) ? shiftNames[row] : "";
                info.required = Config::getMinStaffPerShift();
                info.assigned = block->getStaffCount();
                missingList.append(info);
            }
        }

        // Cleanup accepted grid blocks (owned locally)
        for (int col = 0; col < 7; ++col)
            qDeleteAll(acceptedGrid[col]);

        view->updateManagerMissingShifts(missingList);
    }
    else
    {
        if (currentEmployeeId < 0)
            return;

        QDate today = QDate::currentDate();
        QDate weekStart = Config::getStartOfCurrentWeek(today).addDays(7);
        currentEmployeeRegistrationWeekStart = weekStart;

        if (employeeScheduleLayoutMode == EmployeeScheduleLayoutMode::FullTimeMock)
        {
            // UI-only demo mode. Future role logic can select this mode and replace
            // fullTimeMockStatuses with real data.
            view->enableRegistration(true);
            view->setUpFullTimeGrid(weekStart, fullTimeMockStatuses);
            return;
        }

        bool registrationOpen = (today.dayOfWeek() == Config::getDayOpenRegisShift());
        view->enableRegistration(registrationOpen);
        view->setUpInteractiveGrid(weekStart, Config::getOpenHour(), Config::getCloseHour());

        // Update summary table headers to match the registration week
        view->updateTableHeaders(weekStart);

        // Fetch shift status for coloring the interactive grid
        QMap<int, QList<Shift*>> pendingShifts = model->getRawStaffShifts(currentEmployeeId, weekStart, 0); // 0 = Pending
        QMap<int, QList<Shift*>> acceptedShifts = model->getRawStaffShifts(currentEmployeeId, weekStart, 1); // 1 = Accepted
        QMap<int, QMap<int, ShiftBlock *>> managerGrid = model->getManagerWeeklyGrid(weekStart, 1);

        view->updateStaffInteractiveGridStatus(pendingShifts, acceptedShifts, managerGrid);

        // cleanup managerGrid blocks which are owned locally
        for (int col = 0; col < 7; ++col)
            qDeleteAll(managerGrid[col]);

        // Data fetching for staff shifts is now fully handled in ViewSchedule_Control
    }
}

void Schedule_Control::onSaveFullTimeShiftsRequested(
    const QList<QList<int>>& selectedShiftsByDay)
{
    if (!view || employeeScheduleLayoutMode != EmployeeScheduleLayoutMode::FullTimeMock)
        return;

    for (int day = 0; day < fullTimeMockStatuses.size() && day < 7; ++day)
    {
        QSet<int> selectedRows;
        if (day < selectedShiftsByDay.size())
            selectedRows = QSet<int>(selectedShiftsByDay[day].begin(),
                                     selectedShiftsByDay[day].end());

        for (int shift = 0; shift < fullTimeMockStatuses[day].size() && shift < 3; ++shift)
        {
            if (fullTimeMockStatuses[day][shift] == FullTimeShiftStatus::StaffShortage)
                continue;

            fullTimeMockStatuses[day][shift] = selectedRows.contains(shift)
                ? FullTimeShiftStatus::Registered
                : FullTimeShiftStatus::Unregistered;
        }
    }

    if (!currentEmployeeRegistrationWeekStart.isValid())
    {
        currentEmployeeRegistrationWeekStart =
            Config::getStartOfCurrentWeek(QDate::currentDate()).addDays(7);
    }
    view->setUpFullTimeGrid(currentEmployeeRegistrationWeekStart,
                            fullTimeMockStatuses);
    view->showFullTimeSaveFeedback(
        "Đã lưu lịch đăng ký mô phỏng thành công.");
}

// ─────────────────────────────────────────────
// Slot: staff pressed "Luu / Xac Nhan"
// Converts the interactive grid selection into time-ranged shifts and saves.
// ─────────────────────────────────────────────

void Schedule_Control::onSaveGridRequested(const QList<QList<int>>& selectedHoursByDay)
{
    if (!model || !view)
        return;
    if (currentEmployeeId < 0)
        return;

    QDate today = QDate::currentDate();
    QDate weekStart = Config::getStartOfCurrentWeek(today).addDays(7);
    int openHour = Config::getOpenHour();

    // Clear any existing drafts so a second save doesn't duplicate
    model->clearDrafts();

    bool anyAdded = false;

    for (int col = 0; col < 7 && col < selectedHoursByDay.size(); ++col)
    {
        QList<int> rows = selectedHoursByDay[col];
        if (rows.isEmpty())
            continue;

        // Sort rows to detect contiguous blocks
        std::sort(rows.begin(), rows.end());

        QDate shiftDate = weekStart.addDays(col);

        // Group contiguous rows into [startRow, endRow] spans
        int spanStart = rows[0];
        int spanEnd   = rows[0];
        for (int k = 1; k < rows.size(); ++k)
        {
            if (rows[k] == spanEnd + 1)
            {
                spanEnd = rows[k];
            }
            else
            {
                // Flush current span
                QTime startT(openHour + spanStart, 0);
                QTime endT  (openHour + spanEnd + 1, 0);
                if (model->handleAddShiftSubmission(currentEmployeeId, shiftDate, startT, endT))
                    anyAdded = true;
                spanStart = rows[k];
                spanEnd   = rows[k];
            }
        }
        // Flush final span
        QTime startT(openHour + spanStart, 0);
        QTime endT  (openHour + spanEnd + 1, 0);
        if (model->handleAddShiftSubmission(currentEmployeeId, shiftDate, startT, endT))
            anyAdded = true;
    }

    if (!anyAdded)
    {
        view->showError("Không có ca làm nào được chọn hợp lệ.");
        return;
    }

    bool saved = model->saveDraftShiftsToDatabase();
    if (saved)
        view->showSuccess("Đã lưu lịch đăng ký thành công!");
    else
        view->showError("Lỗi kết nối cơ sở dữ liệu! Không thể lưu lịch.");
}

void Schedule_Control::handleGenSchedule()
{
    if (!model || !view)
        return;
    QStringList warnings = model->generateSchedule();

    view->showSuccess("Xếp lịch tự động hoàn tất!");
    if (!warnings.isEmpty())
    {
        view->showError(warnings.join("\n"));
    }

    // Refresh the assign grid
    QMap<int, QMap<int, BlockCounts>> counts =
        model->getAssignBlockCounts(currentAssignMonday);
    view->updateAssignGrid(counts);
}

void Schedule_Control::search()
{
    qDebug() << "Schedule_Control::search() - not yet implemented.";
}

void Schedule_Control::filter()
{
    qDebug() << "Schedule_Control::filter() - not yet implemented.";
}

void Schedule_Control::chooseDate()
{
    qDebug() << "Schedule_Control::chooseDate() - not yet implemented.";
}

void Schedule_Control::handleChangeAlgorithm()
{
    qDebug() << "Schedule_Control::handleChangeAlgorithm() - not yet implemented.";
}

// ─────────────────────────────────────────────
// Manager: shift block clicked -> open popup
// ─────────────────────────────────────────────

void Schedule_Control::onShiftBlockClicked(int col, int row)
{
    if (!view || !model)
        return;
    if (col < 0 || col >= 7 || row < 0 || row >= 3)
        return;

    QList<PendingShiftInfo> requests = model->getShiftsForBlock(currentAssignMonday, col, row);

    static const QString SHIFT_NAMES[3] = {"Ca Sáng", "Ca Chiều", "Ca Tối"};
    static const QString SHIFT_TIMES[3] = {"07:00 - 12:00", "12:00 - 17:00", "17:00 - 22:00"};

    QString colLabel = currentAssignMonday.addDays(col).toString("dd/MM/yyyy");
    QString shiftLabel = QString("%1 (%2) — %3")
                             .arg(SHIFT_NAMES[row], SHIFT_TIMES[row], colLabel);

    view->showShiftRequestsDialog(requests, shiftLabel);
}

// ─────────────────────────────────────────────
// Manager: approve/decline a shift request
// ─────────────────────────────────────────────

void Schedule_Control::onApproveShift(int shiftId)
{
    if (!model || !view)
        return;
    if (model->approveShift(shiftId))
    {
        // Refresh the assign grid to reflect updated counts
        QMap<int, QMap<int, BlockCounts>> counts =
            model->getAssignBlockCounts(currentAssignMonday);
        view->updateAssignGrid(counts);
    }
    else
    {
        view->showError("Không thể duyệt ca làm vui lòng thử lại.");
    }
}

void Schedule_Control::onDeclineShift(int shiftId)
{
    if (!model || !view)
        return;
    if (model->declineShift(shiftId))
    {
        // Refresh the assign grid to reflect updated counts
        QMap<int, QMap<int, BlockCounts>> counts =
            model->getAssignBlockCounts(currentAssignMonday);
        view->updateAssignGrid(counts);
    }
    else
    {
        view->showError("Không thể từ chối ca làm, vui lòng thử lại.");
    }
}

// ─────────────────────────────────────────────
// Manager: "Xac Nhan" — confirm current state
// ─────────────────────────────────────────────

void Schedule_Control::onConfirmRequested()
{
    // Refresh and show current state as confirmation
    load();
    view->showSuccess("Đã xác nhận lịch làm việc!");
}

// ─────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────

QDate Schedule_Control::dayStringToDate(const QString &day) const
{
    // Find the index of this day by matching prefix with listDays (0 = Monday)
    int idx = -1;
    for (int i = 0; i < listDays.size(); ++i)
    {
        if (day.startsWith(listDays[i]))
        {
            idx = i;
            break;
        }
    }
    if (idx < 0)
        return QDate(); // invalid

    // Nhan vien dang ky lich lam la cho TUAN SAU
    QDate monday = Config::getStartOfCurrentWeek(QDate::currentDate()).addDays(7);
    return monday.addDays(idx);
}
