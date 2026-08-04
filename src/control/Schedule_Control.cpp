#include "global.h"
#include "control/Schedule_Control.h"
#include "view/Schedule_View.h"
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QListWidget>

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
}

void Schedule_Control::onLeaveRequested()
{
    if (!view || currentEmployeeId < 0)
        return;

    const QDate weekStart = currentEmployeeRegistrationWeekStart.isValid()
        ? currentEmployeeRegistrationWeekStart
        : Config::getStartOfCurrentWeek(QDate::currentDate()).addDays(7);
    const QList<LeaveShiftOption> shiftOptions =
        leaveRequestModel.getActiveShiftsForWeek(currentEmployeeId, weekStart);
    if (shiftOptions.isEmpty())
    {
        QMessageBox::information(
            view, QString::fromUtf8("Chưa có ca để xin nghỉ"),
            QString::fromUtf8("Bạn chưa có ca chờ duyệt hoặc đã duyệt trong tuần đăng ký này."));
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
        "QListWidget::item{padding:9px;border-bottom:1px solid #E2E8F0;}"
        "QListWidget::item:selected,QListWidget::item:selected:active,"
        "QListWidget::item:selected:!active{background:transparent;color:#1E293B;}"
        "QPushButton{background:#FFFFFF;color:#334155;border:1px solid #CBD5E1;"
        "border-radius:6px;padding:7px 14px;font-weight:700;}"
        "QPushButton:hover{background:#F1F5F9;}");
    auto *layout = new QVBoxLayout(&dialog);
    auto *shiftLabel = new QLabel(
        QString::fromUtf8("Chọn ca làm của bạn trong tuần để gửi yêu cầu nghỉ cả ngày:"),
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
        // Manager sees and assigns schedule for NEXT WEEK on first load.
        if (!managerWeekInitialized || !currentAssignMonday.isValid())
        {
            currentAssignMonday = Config::getStartOfCurrentWeek(today).addDays(7);
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
                if (blockCounts.required <= blockCounts.accepted && blockCounts.pending == 0)
                {
                    ++staffedShifts;
                    continue;
                }
                MissingShiftInfo info;
                info.dateStr = currentAssignMonday.addDays(col).toString("dd/MM/yyyy");
                info.shiftName = (row < shiftNames.size()) ? shiftNames[row] : "";
                info.required = blockCounts.required;
                info.assigned = blockCounts.accepted;
                info.dayColumn = col;
                info.shiftRow = row;
                missingList.append(info);
                missingSlots += qMax(0, blockCounts.required - blockCounts.accepted);
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
        QDate weekStart = Config::getStartOfCurrentWeek(today).addDays(7);
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

    static const QTime starts[3] = {QTime(7, 0), QTime(12, 0), QTime(17, 0)};
    static const QTime ends[3] = {QTime(12, 0), QTime(17, 0), QTime(22, 0)};
    for (const ManagerScheduleChange &change : preview.changes)
    {
        int day = currentAssignMonday.daysTo(change.date);
        if (day < 0 || day >= 7)
            continue;
        for (int row = 0; row < 3; ++row)
        {
            if (!(change.startTime < ends[row] && change.endTime > starts[row]))
                continue;
            BlockCounts &cell = afterCounts[day][row];
            if (change.type == ManagerScheduleChangeType::Approve)
            {
                cell.pending = qMax(0, cell.pending - 1);
                ++cell.accepted;
            }
            else if (change.type == ManagerScheduleChangeType::Decline)
            {
                cell.pending = qMax(0, cell.pending - 1);
                ++cell.declined;
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
            bool wasMissing = before.accepted < before.required;
            bool stillMissing = after.accepted < after.required;
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
    currentAssignMonday = Config::getStartOfCurrentWeek(QDate::currentDate()).addDays(7);
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
        int required = 0;
        int before = 0;
        int after = 0;
        QStringList actions;
    };
    QMap<QString, ShiftImpact> impacts;
    static const QTime starts[3] = {QTime(7, 0), QTime(12, 0), QTime(17, 0)};
    static const QTime ends[3] = {QTime(12, 0), QTime(17, 0), QTime(22, 0)};
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
                if (!(change.startTime < ends[row] && change.endTime > starts[row]))
                    continue;
                QString key = QString("%1|%2").arg(change.date.toString(Qt::ISODate)).arg(row);
                if (!impacts.contains(key))
                {
                    BlockCounts counts = currentCounts.value(day).value(row);
                    ShiftImpact impact;
                    impact.date = change.date;
                    impact.shiftRow = row;
                    impact.required = counts.required;
                    impact.before = counts.accepted;
                    impact.after = counts.accepted;
                    impacts.insert(key, impact);
                }
                ShiftImpact &impact = impacts[key];
                if (change.type == ManagerScheduleChangeType::Approve ||
                    change.type == ManagerScheduleChangeType::Add)
                    ++impact.after;
                else if (change.type == ManagerScheduleChangeType::Cancel)
                    impact.after = qMax(0, impact.after - 1);
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
        if (impact.required <= 0)
            resultText = "Không đổi định biên";
        else if (impact.after >= impact.required)
            resultText = "Đủ nhân sự";
        else
            resultText = QString("Thiếu %1").arg(impact.required - impact.after);
        reviewTable->setItem(reviewRow, 0, new QTableWidgetItem(shiftText));
        reviewTable->setItem(reviewRow, 1,
                             new QTableWidgetItem(impact.actions.join("\n")));
        reviewTable->setItem(reviewRow, 2, new QTableWidgetItem(impact.required > 0 ? QString("%1/%2").arg(impact.before).arg(impact.required) : "—"));
        reviewTable->setItem(reviewRow, 3, new QTableWidgetItem(impact.required > 0 ? QString("%1/%2").arg(impact.after).arg(impact.required) : "—"));
        QTableWidgetItem *resultItem = new QTableWidgetItem(resultText);
        resultItem->setForeground(impact.required > 0 && impact.after < impact.required
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
void Schedule_Control::onAddEmployeeToShift(int employeeId, QDate date,
                                            QTime startTime, QTime endTime,
                                            const QString &reason)
{
    if (!model || !view)
        return;

    ManagerScheduleChange change;
    change.type = ManagerScheduleChangeType::Add;
    change.employeeId = employeeId;
    change.date = date;
    change.startTime = startTime;
    change.endTime = endTime;
    change.reason = reason;

    if (!date.isValid() || !startTime.isValid() || !endTime.isValid() ||
        startTime >= endTime)
    {
        view->resetManagerAddButton();
        view->showError(QString::fromUtf8("Ngày hoặc khoảng giờ thêm vào không hợp lệ."));
        return;
    }

    const QList<EligibleEmployeeInfo> eligibleEmployees =
        model->getEligibleEmployees(date, startTime, endTime);
    auto employeeIt = std::find_if(
        eligibleEmployees.cbegin(), eligibleEmployees.cend(),
        [employeeId](const EligibleEmployeeInfo &employee) {
            return employee.employeeId == employeeId;
        });
    if (employeeIt == eligibleEmployees.cend())
    {
        view->resetManagerAddButton();
        view->showError(QString::fromUtf8("Không tìm thấy nhân viên để thêm vào ca."));
        return;
    }
    if (!employeeIt->eligible)
    {
        view->resetManagerAddButton();
        view->showError(QString::fromUtf8("Không thể thêm %1: %2")
                            .arg(employeeIt->employeeName, employeeIt->reason));
        return;
    }
    change.employeeName = employeeIt->employeeName;
    change.role = employeeIt->role;

    QList<ManagerScheduleChange> candidateChanges = managerDraftChanges;
    candidateChanges.append(change);
    const QStringList validationErrors =
        model->validateManagerScheduleChanges(candidateChanges);
    if (!validationErrors.isEmpty())
    {
        view->resetManagerAddButton();
        view->showError(validationErrors.join("\n"));
        return;
    }

    managerDraftChanges.erase(
        std::remove_if(managerDraftChanges.begin(), managerDraftChanges.end(),
                       [change](const ManagerScheduleChange &existing)
                       {
                           return existing.type == ManagerScheduleChangeType::Add &&
                                  existing.employeeId == change.employeeId &&
                                  existing.date == change.date &&
                                  existing.startTime == change.startTime &&
                                  existing.endTime == change.endTime;
                       }),
        managerDraftChanges.end());
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

    // Nhan vien dang ky lich lam la cho TUAN SAU
    QDate monday = Config::getStartOfCurrentWeek(QDate::currentDate()).addDays(7);
    return monday.addDays(idx);
}
