#include "global.h"
#include "control/Schedule_Control.h"
#include "view/Schedule_View.h"
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QListWidget>

static QString staffingRoleNameForSchedule(const QString &role)
{
    return Config::displayRoleName(role);
}

static QString staffingDeficitTextForSchedule(const BlockCounts &counts)
{
    QStringList parts;
    const QMap<QString, int> missing = counts.missingByRole();
    for (auto it = missing.constBegin(); it != missing.constEnd(); ++it)
        parts.append(QString("%1 %2")
                         .arg(staffingRoleNameForSchedule(it.key()))
                         .arg(it.value()));
    return parts.join(", ");
}

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

    connect(view, &Schedule_View::requestAddEmployees,
            this, &Schedule_Control::onAddEmployeesToShift);
    connect(view, &Schedule_View::requestRemoveAssignedShift,
            this, &Schedule_Control::onRemoveAssignedShift);
    connect(view, &Schedule_View::requestPreviousManagerWeek,
            this, &Schedule_Control::onPreviousManagerWeek);
    connect(view, &Schedule_View::requestNextManagerWeek,
            this, &Schedule_Control::onNextManagerWeek);
    connect(view, &Schedule_View::requestCurrentManagerWeek,
            this, &Schedule_Control::onCurrentManagerWeek);
    connect(view, &Schedule_View::requestUndoManagerDraft,
            this, &Schedule_Control::onUndoManagerDraft);
    connect(view, &Schedule_View::requestClearManagerDraft,
            this, &Schedule_Control::onClearManagerDraft);
    connect(view, &Schedule_View::requestLeave,
            this, &Schedule_Control::onLeaveRequested);
    connect(view, &Schedule_View::requestLeaveHistory,
            this, &Schedule_Control::onLeaveHistoryRequested);
}

void Schedule_Control::onLeaveRequested()
{
    if (!view || currentEmployeeId < 0)
        return;

    // Leave requests are based on the current calendar week, independently of
    // the registration view (which intentionally edits next week's schedule).
    // Keep only shifts that are still actionable so a Monday/Tuesday shift
    // cannot be selected after it has already started.
    const QDate today = QDate::currentDate();
    const QDate leaveWeekStart = Config::getStartOfCurrentWeek(today);
    const QList<LeaveShiftOption> activeShifts =
        leaveRequestModel.getActiveShiftsForWeek(currentEmployeeId, leaveWeekStart);
    QList<LeaveShiftOption> shiftOptions;
    for (const LeaveShiftOption &option : activeShifts)
    {
        if (option.date >= today)
            shiftOptions.append(option);
    }
    if (shiftOptions.isEmpty())
    {
        QMessageBox::information(
            view, QString::fromUtf8("Chưa có ca để xin nghỉ"),
            QString::fromUtf8("Bạn chưa có ca chờ duyệt hoặc đã duyệt trong tuần này."));
        return;
    }

    QDialog dialog(view);
    dialog.setWindowTitle(QString::fromUtf8("Xin nghỉ phép"));
    dialog.setMinimumWidth(390);
    dialog.setStyleSheet(
        "QDialog{background:#F8FAFC;color:#1E293B;}"
        "QLabel{color:#334155;font-weight:600;}"
        "QListWidget,QPlainTextEdit{background:#FFFFFF;color:#1E293B;"
        "border:1px solid #CBD5E1;border-radius:6px;padding:6px;}"
        "QListWidget::item{padding:9px;border:1px solid transparent;border-bottom:none;}"
        "QListWidget::item:selected,QListWidget::item:selected:active,"
        "QListWidget::item:selected:!active{background:transparent;color:#1E293B;"
        "border:1px solid #2563EB;border-left:4px solid #2563EB;border-radius:6px;}"
        "QPushButton{background:#FFFFFF;color:#334155;border:1px solid #CBD5E1;"
        "border-radius:6px;padding:7px 14px;font-weight:700;}"
        "QPushButton:hover{background:#F1F5F9;}");
    auto *layout = new QVBoxLayout(&dialog);
    auto *shiftLabel = new QLabel(
        QString::fromUtf8("Chọn ca làm còn lại trong tuần này để gửi yêu cầu nghỉ cả ngày:"),
        &dialog);
    shiftLabel->setWordWrap(true);
    layout->addWidget(shiftLabel);
    auto *shiftList = new QListWidget(&dialog);
    shiftList->setSelectionMode(QAbstractItemView::SingleSelection);
    shiftList->setMinimumHeight(145);
    for (const LeaveShiftOption &option : shiftOptions)
    {
        const QString status = option.status == 1
            ? QString::fromUtf8("Đã duyệt") : QString::fromUtf8("Chờ duyệt");
        auto *item = new QListWidgetItem(
            QString("%1  |  %2 - %3  |  %4")
                .arg(option.date.toString("ddd, dd/MM/yyyy"),
                     option.startTime.toString("HH:mm"),
                     option.endTime.toString("HH:mm"), status),
            shiftList);
        item->setData(Qt::UserRole, option.shiftId);
    }
    shiftList->setCurrentRow(0);
    layout->addWidget(shiftList);

    auto *form = new QFormLayout();
    auto *reasonEdit = new QPlainTextEdit(&dialog);
    reasonEdit->setPlaceholderText(QString::fromUtf8("Nhập lý do xin nghỉ..."));
    reasonEdit->setFixedHeight(95);
    form->addRow(QString::fromUtf8("Lý do:"), reasonEdit);
    layout->addLayout(form);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, &dialog);
    QPushButton *submit = buttons->addButton(QString::fromUtf8("Gửi yêu cầu"),
                                               QDialogButtonBox::AcceptRole);
    layout->addWidget(buttons);
    submit->setStyleSheet(
        "QPushButton{background:#2563EB;color:#FFFFFF;border:1px solid #2563EB;}"
        "QPushButton:hover{background:#1D4ED8;}");
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(submit, &QPushButton::clicked, &dialog, [&]() {
        QListWidgetItem *selectedShift = shiftList->currentItem();
        if (!selectedShift)
        {
            QMessageBox::warning(&dialog, QString::fromUtf8("Chưa chọn ca làm"),
                                 QString::fromUtf8("Hãy chọn ca làm cần xin nghỉ."));
            return;
        }
        QString error;
        if (!leaveRequestModel.submitLeaveRequest(currentEmployeeId,
                                                   selectedShift->data(Qt::UserRole).toInt(),
                                                   reasonEdit->toPlainText(), &error)) {
            QMessageBox::warning(&dialog, QString::fromUtf8("Không thể gửi yêu cầu"), error);
            return;
        }
        dialog.accept();
    });

    if (dialog.exec() == QDialog::Accepted)
        view->showSuccess(QString::fromUtf8("Đã gửi yêu cầu nghỉ phép. Vui lòng chờ quản lý duyệt."));
}

void Schedule_Control::onLeaveHistoryRequested()
{
    if (!view || currentEmployeeId < 0)
        return;

    const QList<LeaveRequestInfo> requests =
        leaveRequestModel.getLeaveRequestsForEmployee(currentEmployeeId);
    QDialog dialog(view);
    dialog.setWindowTitle(QString::fromUtf8("Lịch sử yêu cầu nghỉ phép"));
    dialog.resize(760, 400);
    dialog.setStyleSheet(
        "QDialog{background:#F8FAFC;color:#1E293B;}"
        "QTableWidget{background:#FFFFFF;color:#1E293B;border:1px solid #CBD5E1;border-radius:7px;}"
        "QTableWidget::item{background:#FFFFFF;color:#1E293B;border-bottom:1px solid #E2E8F0;padding:7px;}"
        "QTableWidget::item:selected{background:#EFF6FF;color:#1E293B;}"
        "QHeaderView::section{background:#EFF6FF;color:#1E3A8A;border:none;padding:8px;font-weight:700;}"
        "QPushButton{background:#FFFFFF;color:#334155;border:1px solid #CBD5E1;"
        "border-radius:6px;padding:7px 14px;font-weight:700;}"
        "QPushButton:hover{background:#F1F5F9;color:#1E293B;}");
    auto *layout = new QVBoxLayout(&dialog);
    auto *table = new QTableWidget(&dialog);
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({QString::fromUtf8("Ngày nghỉ"),
                                      QString::fromUtf8("Lý do"),
                                      QString::fromUtf8("Trạng thái"),
                                      QString::fromUtf8("Quyết định lúc"),
                                      QString::fromUtf8("Ghi chú quản lý")});
    table->setRowCount(requests.size());
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);

    for (int row = 0; row < requests.size(); ++row) {
        const LeaveRequestInfo &request = requests[row];
        table->setItem(row, 0, new QTableWidgetItem(request.leaveDate.toString("dd/MM/yyyy")));
        table->setItem(row, 1, new QTableWidgetItem(request.reason));
        auto *status = new QTableWidgetItem(
            request.status == "Approved" ? QString::fromUtf8("Đã duyệt")
            : request.status == "Declined" ? QString::fromUtf8("Đã từ chối")
                                           : QString::fromUtf8("Chờ duyệt"));
        status->setForeground(request.status == "Approved" ? QColor("#15803D")
                              : request.status == "Declined" ? QColor("#B91C1C")
                                                             : QColor("#B45309"));
        table->setItem(row, 2, status);
        table->setItem(row, 3, new QTableWidgetItem(
            request.decidedAt.isValid()
                ? request.decidedAt.toLocalTime().toString("dd/MM/yyyy HH:mm") : "-"));
        table->setItem(row, 4, new QTableWidgetItem(
            request.decisionReason.isEmpty() ? "-" : request.decisionReason));
        table->setRowHeight(row, 48);
    }
    layout->addWidget(table);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    dialog.exec();
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

    const QDate registrationToday = QDate::currentDate();
    const QDate registrationWeekStart =
        Config::getStartOfNextWeek(registrationToday);
    if (registrationToday.dayOfWeek() == Config::getDayOpenRegisShift())
    {
        QStringList carryErrors;
        if (!model->ensurePendingCarryForwardForWeek(registrationWeekStart,
                                                     &carryErrors))
            qWarning() << "Weekly shift carry-forward failed:" << carryErrors;
    }

    if (isManager)
    {
        QDate today = QDate::currentDate();
        // Managers can review and change staff schedules on any day.  The
        // configured registration day only limits staff self-registration.
        view->setManagerAssignmentState(true, QDate());
        // Manager assigns the next calendar week, always Monday through Sunday.
        if (!managerWeekInitialized || !currentAssignMonday.isValid())
        {
            currentAssignMonday = Config::getStartOfNextWeek(today);
            managerWeekInitialized = true;
        }

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
        int missingSlots = 0;
        int staffedShifts = 0;
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
                if (!blockCounts.hasShortage() && blockCounts.pending == 0)
                {
                    ++staffedShifts;
                    continue;
                }
                MissingShiftInfo info;
                info.dateStr = currentAssignMonday.addDays(col).toString("dd/MM/yyyy");
                info.shiftName = (row < shiftNames.size()) ? shiftNames[row] : "";
                info.required = blockCounts.required;
                info.assigned = blockCounts.accepted;
                info.missingByRole = blockCounts.missingByRole();
                info.dayColumn = col;
                info.shiftRow = row;
                missingList.append(info);
                missingSlots += blockCounts.missingSlots();
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
                                   managerDraftChanges.size(), missingSlots,
                                   staffedShifts);
        view->updateManagerWeek(currentAssignMonday);
    }
    else
    {
        if (currentEmployeeId < 0)
            return;

        QDate today = QDate::currentDate();
        QDate weekStart = Config::getStartOfNextWeek(today);
        currentEmployeeRegistrationWeekStart = weekStart;

        bool registrationOpen = (today.dayOfWeek() == Config::getDayOpenRegisShift());
        int daysUntilRegistration =
            (Config::getDayOpenRegisShift() - today.dayOfWeek() + 7) % 7;
        QDate nextRegistrationDate = today.addDays(daysUntilRegistration);

        if (employeeScheduleLayoutMode == EmployeeScheduleLayoutMode::FullTimeSchedule)
        {
            fullTimeScheduleStatuses =
                model->getFullTimeScheduleGrid(currentEmployeeId, weekStart);
            view->setUpFullTimeScheduleGrid(weekStart,
                                            fullTimeScheduleStatuses);
            view->setPartTimeRegistrationState(registrationOpen,
                                               nextRegistrationDate);
            return;
        }

        view->setPartTimeRegistrationState(registrationOpen,
                                           nextRegistrationDate);
        view->setUpInteractiveGrid(weekStart, Config::getOpenHour(), Config::getCloseHour());

        // Update summary table headers to match the registration week
        view->updateTableHeaders(weekStart);

        // Fetch shift status for coloring the interactive grid
        QMap<int, QList<Shift *>> pendingShifts = model->getRawStaffShifts(currentEmployeeId, weekStart, 0);  // 0 = Pending
        QMap<int, QList<Shift *>> acceptedShifts = model->getRawStaffShifts(currentEmployeeId, weekStart, 1); // 1 = Accepted
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
    const QList<QList<int>> &selectedShiftsByDay)
{
    if (!model || !view || currentEmployeeId < 0 ||
        employeeScheduleLayoutMode != EmployeeScheduleLayoutMode::FullTimeSchedule)
        return;

    QDate today = QDate::currentDate();
    if (today.dayOfWeek() != Config::getDayOpenRegisShift())
    {
        view->showError("Đăng ký ca làm việc chỉ mở vào ngày được quy định!");
        return;
    }

    QDate weekStart = currentEmployeeRegistrationWeekStart.isValid()
                          ? currentEmployeeRegistrationWeekStart
                          : Config::getStartOfNextWeek(QDate::currentDate());
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
                                  Config::getShiftStartTime(shift),
                                  Config::getShiftEndTime(shift)});
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
    else
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

void Schedule_Control::onSaveGridRequested(const QList<QList<int>> &selectedHoursByDay)
{
    if (!model || !view)
        return;
    if (true)
    {
        if (currentEmployeeId < 0)
            return;

        QDate weekStart = currentEmployeeRegistrationWeekStart.isValid()
                              ? currentEmployeeRegistrationWeekStart
                              : Config::getStartOfNextWeek(QDate::currentDate());
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
            int spanEnd = rows[0];
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
                    spanEnd = rows[k];
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
        else
            view->showError(
                "Không thể cập nhật lịch chờ duyệt. Lịch đã duyệt hoặc lỗi cơ sở "
                "dữ liệu có thể đang ngăn thay đổi này.");
    }
}

void Schedule_Control::handleGenSchedule()
{
    if (!model || !view)
        return;
    if (!managerDraftChanges.isEmpty())
    {
        view->showError(
            "Hãy công bố hoặc xóa bản nháp hiện tại trước khi tạo đề xuất tự động.");
        return;
    }

    QMap<int, QMap<int, BlockCounts>> beforeCounts =
        model->getAssignBlockCounts(currentAssignMonday);
    QMap<int, QMap<int, BlockCounts>> afterCounts = beforeCounts;
    AutoSchedulePreview preview = model->previewGeneratedSchedule(currentAssignMonday);

    for (const ManagerScheduleChange &change : preview.changes)
    {
        int day = currentAssignMonday.daysTo(change.date);
        if (day < 0 || day >= 7)
            continue;
        for (int row = 0; row < 3; ++row)
        {
            if (!(change.startTime < Config::getShiftEndTime(row) &&
                  change.endTime > Config::getShiftStartTime(row)))
                continue;
            BlockCounts &cell = afterCounts[day][row];
            if (!Config::isOperationalRole(change.role))
                continue;
            const QString staffingRole = Config::canonicalRoleName(change.role);
            if (change.type == ManagerScheduleChangeType::Approve)
            {
                cell.adjustStatus(staffingRole, 0, -1);
                cell.adjustStatus(staffingRole, 1, 1);
            }
            else if (change.type == ManagerScheduleChangeType::Decline)
            {
                cell.adjustStatus(staffingRole, 0, -1);
                cell.adjustStatus(staffingRole, -1, 1);
            }
        }
    }

    int resolvedShifts = 0;
    int remainingShortages = 0;
    for (int day = 0; day < 7; ++day)
        for (int row = 0; row < 3; ++row)
        {
            const BlockCounts before = beforeCounts.value(day).value(row);
            const BlockCounts after = afterCounts.value(day).value(row);
            bool wasMissing = before.hasShortage();
            bool stillMissing = after.hasShortage();
            if (wasMissing && !stillMissing)
                ++resolvedShifts;
            if (stillMissing)
                ++remainingShortages;
        }

    QDialog dialog(view);
    dialog.setWindowTitle("Xem trước xếp lịch tự động");
    dialog.setMinimumSize(700, 470);
    dialog.setStyleSheet("QDialog { background:#F8FAFC; }");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QLabel *title = new QLabel("Kết quả đề xuất tự động", &dialog);
    title->setStyleSheet("color:#0F172A;font-size:20px;font-weight:800;");
    QLabel *subtitle = new QLabel(
        QString("Tuần %1 - %2 • Chưa có thay đổi nào được lưu")
            .arg(currentAssignMonday.toString("dd/MM/yyyy"),
                 currentAssignMonday.addDays(6).toString("dd/MM/yyyy")),
        &dialog);
    subtitle->setStyleSheet("color:#64748B;font-size:12px;");
    layout->addWidget(title);
    layout->addWidget(subtitle);

    QHBoxLayout *metrics = new QHBoxLayout();
    auto metric = [&dialog](const QString &label, int value,
                            const QString &background, const QString &color)
    {
        QFrame *card = new QFrame(&dialog);
        card->setStyleSheet(QString("QFrame { background:%1;border-radius:9px; }")
                                .arg(background));
        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        QLabel *valueLabel = new QLabel(QString::number(value), card);
        valueLabel->setStyleSheet(QString("color:%1;font-size:22px;font-weight:800;")
                                      .arg(color));
        QLabel *textLabel = new QLabel(label, card);
        textLabel->setStyleSheet("color:#64748B;font-size:10px;font-weight:700;");
        cardLayout->addWidget(valueLabel);
        cardLayout->addWidget(textLabel);
        return card;
    };
    metrics->addWidget(metric("YÊU CẦU ĐƯỢC DUYỆT", preview.approvedCount,
                              "#ECFDF5", "#047857"));
    metrics->addWidget(metric("YÊU CẦU BỊ TỪ CHỐI", preview.declinedCount,
                              "#FEF2F2", "#B91C1C"));
    metrics->addWidget(metric("CA ĐƯỢC GIẢI QUYẾT", resolvedShifts,
                              "#EFF6FF", "#1D4ED8"));
    metrics->addWidget(metric("CA VẪN CÒN THIẾU", remainingShortages,
                              "#FFFBEB", "#B45309"));
    layout->addLayout(metrics);

    QLabel *warningTitle = new QLabel(
        QString("Cảnh báo và giới hạn (%1)").arg(preview.warnings.size()), &dialog);
    warningTitle->setStyleSheet("color:#334155;font-size:13px;font-weight:800;");
    layout->addWidget(warningTitle);
    QListWidget *warningList = new QListWidget(&dialog);
    warningList->setStyleSheet(
        "QListWidget { background:#FFFFFF;border:1px solid #E2E8F0;"
        "border-radius:8px;padding:6px;color:#92400E; }"
        "QListWidget::item { padding:5px;border-bottom:1px solid #FEF3C7; }");
    if (preview.warnings.isEmpty())
        warningList->addItem("Không phát hiện cảnh báo mới.");
    else
        warningList->addItems(preview.warnings);
    layout->addWidget(warningList, 1);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel,
                                                     Qt::Horizontal, &dialog);
    QPushButton *accept = buttons->addButton("Thêm đề xuất vào bản nháp",
                                             QDialogButtonBox::AcceptRole);
    accept->setEnabled(!preview.changes.isEmpty());
    accept->setStyleSheet(
        "QPushButton { background:#2563EB;color:white;border:none;border-radius:6px;"
        "padding:8px 16px;font-weight:700; }"
        "QPushButton:disabled { background:#CBD5E1;color:#64748B; }");
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (dialog.exec() != QDialog::Accepted)
        return;

    managerDraftChanges = preview.changes;
    view->setManagerDraftStatus(managerDraftChanges.size());
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

void Schedule_Control::onPreviousManagerWeek()
{
    if (!managerDraftChanges.isEmpty())
    {
        view->showError("Hãy công bố hoặc xóa bản nháp trước khi đổi tuần.");
        return;
    }
    if (!currentAssignMonday.isValid())
        return;
    currentAssignMonday = currentAssignMonday.addDays(-7);
    load();
}

void Schedule_Control::onNextManagerWeek()
{
    if (!managerDraftChanges.isEmpty())
    {
        view->showError("Hãy công bố hoặc xóa bản nháp trước khi đổi tuần.");
        return;
    }
    if (!currentAssignMonday.isValid())
        return;
    currentAssignMonday = currentAssignMonday.addDays(7);
    load();
}

void Schedule_Control::onCurrentManagerWeek()
{
    if (!managerDraftChanges.isEmpty())
    {
        view->showError("Hãy công bố hoặc xóa bản nháp trước khi đổi tuần.");
        return;
    }
    currentAssignMonday = Config::getStartOfNextWeek(QDate::currentDate());
    managerWeekInitialized = true;
    load();
}

void Schedule_Control::onUndoManagerDraft()
{
    if (managerDraftChanges.isEmpty() || !view)
        return;
    managerDraftChanges.removeLast();
    view->setManagerDraftStatus(managerDraftChanges.size());
    if (selectedManagerDay >= 0 && selectedManagerShift >= 0)
        onShiftBlockClicked(selectedManagerDay, selectedManagerShift);
}

void Schedule_Control::onClearManagerDraft()
{
    if (managerDraftChanges.isEmpty() || !view)
        return;
    if (QMessageBox::question(view, "Xóa bản nháp",
                              "Xóa toàn bộ thay đổi chưa công bố?",
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) != QMessageBox::Yes)
        return;
    managerDraftChanges.clear();
    view->setManagerDraftStatus(0);
    if (selectedManagerDay >= 0 && selectedManagerShift >= 0)
        onShiftBlockClicked(selectedManagerDay, selectedManagerShift);
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
    selectedManagerDay = col;
    selectedManagerShift = row;

    QList<PendingShiftInfo> requests = model->getShiftsForBlock(currentAssignMonday, col, row);

    static const QString SHIFT_NAMES[3] = {"Ca Sáng", "Ca Chiều", "Ca Tối"};

    QString colLabel = currentAssignMonday.addDays(col).toString("dd/MM/yyyy");
    QString shiftLabel = QString("%1 (%2) — %3")
                             .arg(SHIFT_NAMES[row],
                                  Config::getShiftTimeLabel(row), colLabel);

    const QTime blockStart = Config::getShiftStartTime(row);
    const QTime blockEnd = Config::getShiftEndTime(row);
    QList<EligibleEmployeeInfo> eligible =
        model->getEligibleEmployees(currentAssignMonday.addDays(col),
                                    blockStart, blockEnd);
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
    change.date = request.date;
    change.startTime = request.startTime;
    change.endTime = request.endTime;
    change.reason = "Manager approval";
    managerDraftChanges.erase(
        std::remove_if(managerDraftChanges.begin(), managerDraftChanges.end(),
                       [change](const ManagerScheduleChange &existing)
                       {
                           return change.shiftId > 0 && existing.shiftId == change.shiftId;
                       }),
        managerDraftChanges.end());
    managerDraftChanges.append(change);
    view->setManagerDraftStatus(managerDraftChanges.size());
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
    change.date = request.date;
    change.startTime = request.startTime;
    change.endTime = request.endTime;
    change.reason = "Manager decline";
    managerDraftChanges.erase(
        std::remove_if(managerDraftChanges.begin(), managerDraftChanges.end(),
                       [change](const ManagerScheduleChange &existing)
                       {
                           return change.shiftId > 0 && existing.shiftId == change.shiftId;
                       }),
        managerDraftChanges.end());
    managerDraftChanges.append(change);
    view->setManagerDraftStatus(managerDraftChanges.size());
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
    if (!model || !view)
        return;
    if (managerDraftChanges.isEmpty())
    {
        QMessageBox::information(view, "Bản nháp trống",
                                 "Chưa có thay đổi nào để công bố.");
        return;
    }

    QStringList validationErrors =
        model->validateManagerScheduleChanges(managerDraftChanges);
    QMap<int, QMap<int, BlockCounts>> currentCounts =
        model->getAssignBlockCounts(currentAssignMonday);

    struct ShiftImpact
    {
        QDate date;
        int shiftRow = -1;
        BlockCounts before;
        BlockCounts after;
        QStringList actions;
    };
    QMap<QString, ShiftImpact> impacts;
    static const QString shiftNames[3] = {"Ca Sáng", "Ca Chiều", "Ca Tối"};

    for (const ManagerScheduleChange &change : managerDraftChanges)
    {
        QString action;
        switch (change.type)
        {
        case ManagerScheduleChangeType::Approve:
            action = "Duyệt";
            break;
        case ManagerScheduleChangeType::Decline:
            action = "Từ chối";
            break;
        case ManagerScheduleChangeType::Add:
            action = "Thêm";
            break;
        case ManagerScheduleChangeType::Cancel:
            action = "Gỡ";
            break;
        }
        QString employee = change.employeeName.isEmpty()
                               ? QString("ID %1").arg(change.employeeId)
                               : change.employeeName;
        bool mapped = false;
        int day = currentAssignMonday.daysTo(change.date);
        if (day >= 0 && day < 7 && change.startTime.isValid() && change.endTime.isValid())
        {
            for (int row = 0; row < 3; ++row)
            {
                if (!(change.startTime < Config::getShiftEndTime(row) &&
                      change.endTime > Config::getShiftStartTime(row)))
                    continue;
                QString key = QString("%1|%2").arg(change.date.toString(Qt::ISODate)).arg(row);
                if (!impacts.contains(key))
                {
                    BlockCounts counts = currentCounts.value(day).value(row);
                    ShiftImpact impact;
                    impact.date = change.date;
                    impact.shiftRow = row;
                    impact.before = counts;
                    impact.after = counts;
                    impacts.insert(key, impact);
                }
                ShiftImpact &impact = impacts[key];
                if (Config::isOperationalRole(change.role))
                {
                    const QString staffingRole =
                        Config::canonicalRoleName(change.role);
                    if (change.type == ManagerScheduleChangeType::Approve)
                    {
                        impact.after.adjustStatus(staffingRole, 0, -1);
                        impact.after.adjustStatus(staffingRole, 1, 1);
                    }
                    else if (change.type == ManagerScheduleChangeType::Decline)
                    {
                        impact.after.adjustStatus(staffingRole, 0, -1);
                        impact.after.adjustStatus(staffingRole, -1, 1);
                    }
                    else if (change.type == ManagerScheduleChangeType::Add)
                        impact.after.adjustStatus(staffingRole, 1, 1);
                    else if (change.type == ManagerScheduleChangeType::Cancel)
                    {
                        impact.after.adjustStatus(staffingRole, 1, -1);
                        impact.after.adjustStatus(staffingRole, -2, 1);
                    }
                }
                impact.actions << QString("%1 %2").arg(action, employee);
                mapped = true;
            }
        }
        if (!mapped)
        {
            QString key = QString("unmapped|%1").arg(impacts.size());
            ShiftImpact impact;
            impact.date = change.date;
            impact.actions << QString("%1 %2").arg(action, employee);
            impacts.insert(key, impact);
        }
    }

    QDialog review(view);
    review.setWindowTitle("Xem lại và công bố lịch");
    review.setMinimumSize(820, 520);
    review.setStyleSheet("QDialog { background:#F8FAFC; }");
    QVBoxLayout *reviewLayout = new QVBoxLayout(&review);
    QLabel *reviewTitle = new QLabel("Xem lại thay đổi trước khi công bố", &review);
    reviewTitle->setStyleSheet("color:#0F172A;font-size:20px;font-weight:800;");
    QLabel *reviewSubtitle = new QLabel(
        QString("%1 thay đổi • %2 ca bị ảnh hưởng")
            .arg(managerDraftChanges.size())
            .arg(impacts.size()),
        &review);
    reviewSubtitle->setStyleSheet("color:#64748B;font-size:12px;");
    reviewLayout->addWidget(reviewTitle);
    reviewLayout->addWidget(reviewSubtitle);

    if (!validationErrors.isEmpty())
    {
        QLabel *validation = new QLabel(
            QString("Không thể công bố:\n• %1")
                .arg(validationErrors.join("\n• ")),
            &review);
        validation->setWordWrap(true);
        validation->setStyleSheet(
            "background:#FEF2F2;color:#B91C1C;border:1px solid #FECACA;"
            "border-radius:8px;padding:10px;font-size:11px;font-weight:600;");
        reviewLayout->addWidget(validation);
    }

    QTableWidget *reviewTable = new QTableWidget(impacts.size(), 5, &review);
    reviewTable->setHorizontalHeaderLabels(
        {"CA LÀM", "THAY ĐỔI", "TRƯỚC", "SAU", "KẾT QUẢ"});
    reviewTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    reviewTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    reviewTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    reviewTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    reviewTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    reviewTable->verticalHeader()->setVisible(false);
    reviewTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    reviewTable->setSelectionMode(QAbstractItemView::NoSelection);
    reviewTable->setWordWrap(true);
    reviewTable->setStyleSheet(
        "QTableWidget { background:#FFFFFF;color:#1E293B;"
        "border:1px solid #E2E8F0;border-radius:8px; }"
        "QHeaderView::section { background:#EFF6FF;color:#1E3A8A;"
        "font-weight:700;padding:8px;border:none; }"
        "QTableWidget::item { color:#1E293B;padding:8px;"
        "border-bottom:1px solid #E2E8F0; }");
    int reviewRow = 0;
    for (const ShiftImpact &impact : impacts)
    {
        QString shiftText = impact.shiftRow >= 0
                                ? QString("%1\n%2").arg(impact.date.toString("dd/MM/yyyy"),
                                                        shiftNames[impact.shiftRow])
                                : (impact.date.isValid() ? impact.date.toString("dd/MM/yyyy")
                                                         : "Chưa xác định");
        QString resultText;
        if (impact.after.required <= 0)
            resultText = "Không đổi định biên";
        else if (!impact.after.hasShortage())
            resultText = "Đủ nhân sự";
        else
            resultText = QString("Thiếu %1 (%2)")
                             .arg(impact.after.missingSlots())
                             .arg(staffingDeficitTextForSchedule(impact.after));
        reviewTable->setItem(reviewRow, 0, new QTableWidgetItem(shiftText));
        reviewTable->setItem(reviewRow, 1,
                             new QTableWidgetItem(impact.actions.join("\n")));
        reviewTable->setItem(reviewRow, 2, new QTableWidgetItem(
            impact.before.required > 0
                ? QString("%1/%2").arg(impact.before.accepted).arg(impact.before.required)
                : "—"));
        reviewTable->setItem(reviewRow, 3, new QTableWidgetItem(
            impact.after.required > 0
                ? QString("%1/%2").arg(impact.after.accepted).arg(impact.after.required)
                : "—"));
        QTableWidgetItem *resultItem = new QTableWidgetItem(resultText);
        resultItem->setForeground(impact.after.required > 0 && impact.after.hasShortage()
                                      ? QColor("#B91C1C")
                                      : QColor("#047857"));
        reviewTable->setItem(reviewRow, 4, resultItem);
        reviewTable->setRowHeight(reviewRow, qMax(52, 24 + impact.actions.size() * 18));
        ++reviewRow;
    }
    reviewLayout->addWidget(reviewTable, 1);

    QDialogButtonBox *reviewButtons = new QDialogButtonBox(
        QDialogButtonBox::Cancel, Qt::Horizontal, &review);
    QPushButton *publishButton = reviewButtons->addButton(
        "Công bố lịch", QDialogButtonBox::AcceptRole);
    if (QPushButton *cancelButton = reviewButtons->button(QDialogButtonBox::Cancel))
    {
        cancelButton->setStyleSheet(
            "QPushButton { background:#FFFFFF;color:#475569;"
            "border:1px solid #CBD5E1;border-radius:6px;"
            "padding:8px 18px;font-weight:600; }"
            "QPushButton:hover { background:#F8FAFC;color:#1E293B; }"
            "QPushButton:pressed { background:#F1F5F9; }");
    }
    publishButton->setEnabled(validationErrors.isEmpty());
    publishButton->setStyleSheet(
        "QPushButton { background:#16A34A;color:white;border:none;border-radius:6px;"
        "padding:8px 18px;font-weight:700; }"
        "QPushButton:disabled { background:#CBD5E1;color:#64748B; }");
    connect(reviewButtons, &QDialogButtonBox::accepted, &review, &QDialog::accept);
    connect(reviewButtons, &QDialogButtonBox::rejected, &review, &QDialog::reject);
    reviewLayout->addWidget(reviewButtons);
    if (review.exec() != QDialog::Accepted)
        return;

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
void Schedule_Control::onAddEmployeesToShift(
    QDate date, QTime blockStart, QTime blockEnd,
    const QList<ManagerEmployeeSelection> &selections)
{
    if (!model || !view)
        return;

    if (!date.isValid() || !blockStart.isValid() || !blockEnd.isValid() ||
        blockStart >= blockEnd || selections.isEmpty())
    {
        view->resetManagerAddButton();
        view->showError(QString::fromUtf8(
            "Ngày, khung giờ hoặc danh sách nhân viên không hợp lệ."));
        return;
    }

    QList<ManagerScheduleChange> additions;
    QStringList selectionErrors;
    QSet<int> selectedEmployeeIds;
    for (const ManagerEmployeeSelection &selection : selections)
    {
        if (selection.employeeId <= 0 ||
            selectedEmployeeIds.contains(selection.employeeId))
        {
            selectionErrors.append(QString::fromUtf8(
                "Danh sách có nhân viên trùng hoặc không hợp lệ."));
            continue;
        }
        selectedEmployeeIds.insert(selection.employeeId);

        const QList<EligibleEmployeeInfo> blockEligibility =
            model->getEligibleEmployees(date, blockStart, blockEnd);
        auto profileIt = std::find_if(
            blockEligibility.cbegin(), blockEligibility.cend(),
            [selection](const EligibleEmployeeInfo &employee) {
                return employee.employeeId == selection.employeeId;
            });
        if (profileIt == blockEligibility.cend())
        {
            selectionErrors.append(QString::fromUtf8(
                "Không tìm thấy nhân viên ID %1.").arg(selection.employeeId));
            continue;
        }

        const bool isManagerRole =
            profileIt->role.compare("Manager", Qt::CaseInsensitive) == 0 ||
            profileIt->role.compare("Manage", Qt::CaseInsensitive) == 0;
        if (isManagerRole)
        {
            selectionErrors.append(QString::fromUtf8(
                "Không thể thêm quản lý vào ca bằng chức năng thêm nhân viên."));
            continue;
        }
        const bool lockedToBlock = profileIt->isFixedSalary;
        QTime requestedStart = lockedToBlock ? blockStart : selection.startTime;
        QTime requestedEnd = lockedToBlock ? blockEnd : selection.endTime;
        if (!requestedStart.isValid() || !requestedEnd.isValid() ||
            requestedStart >= requestedEnd)
        {
            selectionErrors.append(QString::fromUtf8(
                "%1: giờ bắt đầu phải nhỏ hơn giờ kết thúc.")
                .arg(selection.employeeName));
            continue;
        }

        const QList<EligibleEmployeeInfo> currentEligibility =
            model->getEligibleEmployees(date, requestedStart, requestedEnd);
        auto employeeIt = std::find_if(
            currentEligibility.cbegin(), currentEligibility.cend(),
            [selection](const EligibleEmployeeInfo &employee) {
                return employee.employeeId == selection.employeeId;
            });
        if (employeeIt == currentEligibility.cend())
        {
            selectionErrors.append(QString::fromUtf8(
                "Không tìm thấy nhân viên ID %1.").arg(selection.employeeId));
            continue;
        }
        if (!employeeIt->eligible)
        {
            selectionErrors.append(QString::fromUtf8("Không thể thêm %1: %2")
                                       .arg(employeeIt->employeeName,
                                            employeeIt->reason));
            continue;
        }

        ManagerScheduleChange change;
        change.type = ManagerScheduleChangeType::Add;
        change.employeeId = employeeIt->employeeId;
        change.employeeName = employeeIt->employeeName;
        change.role = employeeIt->role;
        change.date = date;
        change.startTime = requestedStart;
        change.endTime = requestedEnd;
        change.reason = employeeIt->isFixedSalary
            ? QString::fromUtf8("Quản lý bổ sung nhân sự cố định")
            : QString::fromUtf8("Quản lý bổ sung nhân sự theo giờ");
        additions.append(change);
    }

    if (!selectionErrors.isEmpty())
    {
        view->resetManagerAddButton();
        view->showError(selectionErrors.join("\n"));
        return;
    }

    QList<ManagerScheduleChange> candidateChanges = managerDraftChanges;
    candidateChanges.append(additions);
    const QStringList validationErrors =
        model->validateManagerScheduleChanges(candidateChanges);
    if (!validationErrors.isEmpty())
    {
        view->resetManagerAddButton();
        view->showError(validationErrors.join("\n"));
        return;
    }

    managerDraftChanges.append(additions);
    view->setManagerDraftStatus(managerDraftChanges.size());
}

void Schedule_Control::onRemoveAssignedShift(int shiftId, int employeeId,
                                             const QString &reason)
{
    if (!model || !view)
        return;
    ManagerScheduleChange change;
    change.type = ManagerScheduleChangeType::Cancel;
    change.shiftId = shiftId;
    change.employeeId = employeeId;
    change.reason = reason;
    if (selectedManagerDay >= 0 && selectedManagerShift >= 0)
    {
        const QList<PendingShiftInfo> requests = model->getShiftsForBlock(
            currentAssignMonday, selectedManagerDay, selectedManagerShift);
        for (const PendingShiftInfo &request : requests)
            if (request.shiftId == shiftId)
            {
                change.employeeName = request.employeeName;
                change.role = request.role;
                change.date = request.date;
                change.startTime = request.startTime;
                change.endTime = request.endTime;
                break;
            }
    }
    managerDraftChanges.erase(
        std::remove_if(managerDraftChanges.begin(), managerDraftChanges.end(),
                       [shiftId](const ManagerScheduleChange &existing)
                       {
                           return shiftId > 0 && existing.shiftId == shiftId;
                       }),
        managerDraftChanges.end());
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

    // Employee registration is always for next Monday through Sunday.
    QDate monday = Config::getStartOfNextWeek(QDate::currentDate());
    return monday.addDays(idx);
}
