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

    connect(view, &Schedule_View::requestSaveFullTimeSchedule,
            this, &Schedule_Control::onSaveFullTimeScheduleRequested);

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

    connect(view, &Schedule_View::requestAddEmployee,
            this, &Schedule_Control::onAddEmployeeToShift);
    connect(view, &Schedule_View::requestRemoveAssignedShift,
            this, &Schedule_Control::onRemoveAssignedShift);
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

    User *currentUser = SessionManager::getInstance()->getCurrentUser();
    bool isManager = currentUser && currentUser->getRole() == "Manager";
    view->setManagerMode(isManager);

    if (!isManager && currentUser)
    {
        employeeScheduleLayoutMode = scheduleLayoutModeForPayType(
            currentUser->getIsFixedSalary());
    }

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
                BlockCounts blockCounts = counts.value(col).value(row);
                if (blockCounts.required <= blockCounts.accepted && blockCounts.pending == 0)
                    continue;
                MissingShiftInfo info;
                info.dateStr = currentAssignMonday.addDays(col).toString("dd/MM/yyyy");
                info.shiftName = (row < shiftNames.size()) ? shiftNames[row] : "";
                info.required = blockCounts.required;
                info.assigned = blockCounts.accepted;
                missingList.append(info);
            }
        }

        // Cleanup accepted grid blocks (owned locally)
        for (int col = 0; col < 7; ++col)
            qDeleteAll(acceptedGrid[col]);

        int pendingTotal = 0;
        for (const auto &day : counts)
            for (const auto &cell : day)
                pendingTotal += cell.pending;
        view->updateManagerMissingShifts(missingList);
        view->updateManagerSummary(21, missingList.size(), pendingTotal,
                                   managerDraftChanges.size());
    }
    else
    {
        if (currentEmployeeId < 0)
            return;

        QDate today = QDate::currentDate();
        QDate weekStart = Config::getStartOfCurrentWeek(today).addDays(7);
        currentEmployeeRegistrationWeekStart = weekStart;

        if (employeeScheduleLayoutMode == EmployeeScheduleLayoutMode::FullTimeSchedule)
        {
            view->enableRegistration(true);
            fullTimeScheduleStatuses =
                model->getFullTimeScheduleGrid(currentEmployeeId, weekStart);
            view->setUpFullTimeScheduleGrid(weekStart,
                                            fullTimeScheduleStatuses);
            return;
        }

        bool registrationOpen = (today.dayOfWeek() == Config::getDayOpenRegisShift());
        int daysUntilRegistration =
            (Config::getDayOpenRegisShift() - today.dayOfWeek() + 7) % 7;
        QDate nextRegistrationDate = today.addDays(daysUntilRegistration);
        view->setPartTimeRegistrationState(registrationOpen,
                                           nextRegistrationDate);
        view->setUpInteractiveGrid(weekStart, Config::getOpenHour(), Config::getCloseHour());

        // Update summary table headers to match the registration week
        view->updateTableHeaders(weekStart);

        // Fetch shift status for coloring the interactive grid
        QMap<int, QList<Shift*>> pendingShifts = model->getRawStaffShifts(currentEmployeeId, weekStart, 0); // 0 = Pending
        QMap<int, QList<Shift*>> acceptedShifts = model->getRawStaffShifts(currentEmployeeId, weekStart, 1); // 1 = Accepted
        QMap<int, QMap<int, ShiftBlock *>> managerGrid = model->getManagerWeeklyGrid(weekStart, 1);

        view->updateStaffInteractiveGridStatus(pendingShifts, acceptedShifts, managerGrid);

        // Raw shift objects and manager-grid blocks are owned by this load call.
        for (int col = 0; col < 7; ++col)
        {
            qDeleteAll(pendingShifts[col]);
            qDeleteAll(acceptedShifts[col]);
        }
        for (int col = 0; col < 7; ++col)
            qDeleteAll(managerGrid[col]);

        // Data fetching for staff shifts is now fully handled in ViewSchedule_Control
    }
}

void Schedule_Control::onSaveFullTimeScheduleRequested(
    const QList<QList<int>>& selectedShiftsByDay)
{
    if (!model || !view || currentEmployeeId < 0 ||
        employeeScheduleLayoutMode != EmployeeScheduleLayoutMode::FullTimeSchedule)
        return;

    static const QTime shiftStarts[3] = {
        QTime(7, 0), QTime(12, 0), QTime(17, 0)};
    static const QTime shiftEnds[3] = {
        QTime(12, 0), QTime(17, 0), QTime(22, 0)};

    QDate weekStart = currentEmployeeRegistrationWeekStart.isValid()
        ? currentEmployeeRegistrationWeekStart
        : Config::getStartOfCurrentWeek(QDate::currentDate()).addDays(7);
    QList<StaffShiftRegistration> registrations;
    for (int day = 0; day < selectedShiftsByDay.size() && day < 7; ++day)
    {
        QSet<int> uniqueRows(selectedShiftsByDay[day].begin(),
                             selectedShiftsByDay[day].end());
        for (int shift : uniqueRows)
        {
            if (shift < 0 || shift >= 3)
                continue;
            registrations.append({weekStart.addDays(day),
                                  shiftStarts[shift],
                                  shiftEnds[shift]});
        }
    }

    if (model->replacePendingShiftsForWeek(currentEmployeeId,
                                           weekStart,
                                           registrations))
    {
        load();
        view->showFullTimeSaveFeedback(
            "Đã lưu lịch đăng ký chờ duyệt thành công.");
    }
    if (false)
    {
        view->showError(
            "Không thể cập nhật lịch toàn thời gian. Ca đã duyệt hoặc lỗi "
            "cơ sở dữ liệu có thể đang ngăn thay đổi này.");
    }
}

// ─────────────────────────────────────────────
// Slot: staff pressed "Luu / Xac Nhan"
// Converts the interactive grid selection into time-ranged shifts and saves.
// ─────────────────────────────────────────────

void Schedule_Control::onSaveGridRequested(const QList<QList<int>>& selectedHoursByDay)
{
    if (!model || !view)
        return;
    if (true)
    {
    if (currentEmployeeId < 0)
        return;

    QDate weekStart = currentEmployeeRegistrationWeekStart.isValid()
        ? currentEmployeeRegistrationWeekStart
        : Config::getStartOfCurrentWeek(QDate::currentDate()).addDays(7);
    int openHour = Config::getOpenHour();
    int rowCount = Config::getCloseHour() - openHour;
    QList<StaffShiftRegistration> registrations;

    for (int col = 0; col < 7 && col < selectedHoursByDay.size(); ++col)
    {
        QList<int> rows = selectedHoursByDay[col];
        rows.erase(std::remove_if(rows.begin(), rows.end(),
                                  [rowCount](int row)
                                  { return row < 0 || row >= rowCount; }),
                   rows.end());
        std::sort(rows.begin(), rows.end());
        rows.erase(std::unique(rows.begin(), rows.end()), rows.end());
        if (rows.isEmpty())
            continue;

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
                registrations.append({shiftDate,
                                      QTime(openHour + spanStart, 0),
                                      QTime(openHour + spanEnd + 1, 0)});
                spanStart = rows[k];
                spanEnd   = rows[k];
            }
        }
        registrations.append({shiftDate,
                              QTime(openHour + spanStart, 0),
                              QTime(openHour + spanEnd + 1, 0)});
    }

    bool saved = model->replacePendingShiftsForWeek(
        currentEmployeeId, weekStart, registrations);
    if (saved)
    {
        view->showSuccess("Đã lưu lịch đăng ký thành công!");
        load();
    }
    }
        view->showError(
            "Không thể cập nhật lịch chờ duyệt. Lịch đã duyệt hoặc lỗi cơ sở "
            "dữ liệu có thể đang ngăn thay đổi này.");
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

    QList<EligibleEmployeeInfo> eligible =
        model->getEligibleEmployees(currentAssignMonday.addDays(col),
                                    QTime(7 + row * 5, 0), QTime(12 + row * 5, 0));
    QTime blockStart(7 + row * 5, 0);
    QTime blockEnd(12 + row * 5, 0);
    view->showShiftRequestsDialog(requests, shiftLabel, eligible,
                                  currentAssignMonday.addDays(col),
                                  blockStart, blockEnd);
}

// ─────────────────────────────────────────────
// Manager: approve/decline a shift request
// ─────────────────────────────────────────────

void Schedule_Control::onApproveShift(PendingShiftInfo request)
{
    if (!model || !view)
        return;
    ManagerScheduleChange change;
    change.type = ManagerScheduleChangeType::Approve;
    change.shiftId = request.shiftId;
    change.employeeId = request.employeeId;
    change.employeeName = request.employeeName;
    change.role = request.role;
    change.startTime = request.startTime;
    change.endTime = request.endTime;
    change.reason = "Manager approval";
    managerDraftChanges.append(change);
    view->setManagerDraftStatus(managerDraftChanges.size());
    view->showSuccess("Đã thêm duyệt vào bản nháp. Hãy xác nhận lịch để lưu.");
}

void Schedule_Control::onDeclineShift(PendingShiftInfo request)
{
    if (!model || !view)
        return;
    const int shiftId = request.shiftId;
    ManagerScheduleChange change;
    change.type = ManagerScheduleChangeType::Decline;
    change.shiftId = request.shiftId;
    change.employeeId = request.employeeId;
    change.employeeName = request.employeeName;
    change.role = request.role;
    change.startTime = request.startTime;
    change.endTime = request.endTime;
    change.reason = "Manager decline";
    managerDraftChanges.append(change);
    view->setManagerDraftStatus(managerDraftChanges.size());
    view->showSuccess("Đã thêm từ chối vào bản nháp. Hãy xác nhận lịch để lưu.");
    return;
#if 0
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
#endif
}

// ─────────────────────────────────────────────
// Manager: "Xac Nhan" — confirm current state
// ─────────────────────────────────────────────

void Schedule_Control::onConfirmRequested()
{
    if (!managerDraftChanges.isEmpty())
    {
        QStringList summary;
        for (const ManagerScheduleChange &change : managerDraftChanges)
        {
            QString action;
            switch (change.type)
            {
            case ManagerScheduleChangeType::Approve: action = "Duyệt"; break;
            case ManagerScheduleChangeType::Decline: action = "Từ chối"; break;
            case ManagerScheduleChangeType::Add: action = "Thêm"; break;
            case ManagerScheduleChangeType::Cancel: action = "Gỡ"; break;
            }
            QString employee = change.employeeName.isEmpty()
                ? QString("ID %1").arg(change.employeeId) : change.employeeName;
            summary << QString("• %1: %2 (%3)").arg(action, employee, change.reason);
        }
        if (QMessageBox::question(view, "Xem thay đổi & xác nhận",
                                  summary.join("\n"), QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::Yes) != QMessageBox::Yes)
            return;
    }
    QStringList errors;
    if (!model->applyManagerScheduleChanges(managerDraftChanges, &errors))
    {
        view->showError(errors.isEmpty() ? "Không thể lưu thay đổi lịch." : errors.join("\n"));
        return;
    }
    managerDraftChanges.clear();
    // Refresh and show current state as confirmation
    load();
    view->setManagerDraftStatus(0);
    view->showSuccess("Đã xác nhận lịch làm việc!");
}

// ─────────────────────────────────────────────
void Schedule_Control::onAddEmployeeToShift(int employeeId, QDate date,
                                             QTime startTime, QTime endTime,
                                             const QString &reason)
{
    ManagerScheduleChange change;
    change.type = ManagerScheduleChangeType::Add;
    change.employeeId = employeeId;
    change.date = date;
    change.startTime = startTime;
    change.endTime = endTime;
    change.reason = reason;
    managerDraftChanges.append(change);
    view->setManagerDraftStatus(managerDraftChanges.size());
}

void Schedule_Control::onRemoveAssignedShift(int shiftId, int employeeId,
                                               const QString &reason)
{
    ManagerScheduleChange change;
    change.type = ManagerScheduleChangeType::Cancel;
    change.shiftId = shiftId;
    change.employeeId = employeeId;
    change.reason = reason;
    managerDraftChanges.append(change);
    view->setManagerDraftStatus(managerDraftChanges.size());
}

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
