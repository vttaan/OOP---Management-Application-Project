#include "ViewSchedule_Control.h"

#include "utils/SessionManage.h"
#include "utils/Config.h"
#include "core/User.h"
ViewSchedule_Control::ViewSchedule_Control(QObject *parent)
    : QObject(parent), view(nullptr), model(new Schedule_Model()), currentEmployeeId(-1) {}

ViewSchedule_Control::~ViewSchedule_Control() {
    delete model;
    for (int col = 0; col < 7; ++col) {
        if (scheduleGrid.contains(col)) {
            qDeleteAll(scheduleGrid[col]);
        }
    }
    scheduleGrid.clear();
}

void ViewSchedule_Control::setEmployeeId(short int id) {
    currentEmployeeId = id;
}

void ViewSchedule_Control::setView(ViewSchedule_View* v) {
    view = v;
    if (!view) return;

    connect(view, &ViewSchedule_View::requestPrevWeek, this, &ViewSchedule_Control::onPrevWeek);
    connect(view, &ViewSchedule_View::requestNextWeek, this, &ViewSchedule_Control::onNextWeek);
    connect(view, &ViewSchedule_View::requestCurrentWeek, this, &ViewSchedule_Control::onCurrentWeek);
    connect(view, &ViewSchedule_View::shiftClicked, this, &ViewSchedule_Control::onShiftClicked);
    
    // Connect replace signals
    connect(view, &ViewSchedule_View::requestShowReplacements, this, &ViewSchedule_Control::onShowReplacementsRequested);
    connect(view, &ViewSchedule_View::requestConfirmReplacement, this, &ViewSchedule_Control::onConfirmReplacement);
}

void ViewSchedule_Control::load() {
    if (!view || currentEmployeeId < 0) return;

    currentViewMonday = Config::getStartOfCurrentWeek(QDate::currentDate());
    loadData();
}


void ViewSchedule_Control::loadData() {
    if (!this->currentSession || !this->currentSession->getCurrentUser()) {
        qDebug() << "ViewSchedule_Control::loadData() - No session or user found";
        return;
    }
    
    if (this->currentSession->checkPermission("Manager") || 
        this->currentSession->checkPermission("Admin")) {
        loadManagerSchedule();
    } else {
        loadStaffSchedule();
    }
}

void ViewSchedule_Control::loadStaffSchedule() {
    view->setManagerFeaturesVisible(false);

    // ── Approved shifts (bottom table) ───────────────────────────────────────
    QMap<int, QList<Shift*>> approvedShifts = model->getRawStaffShifts(currentEmployeeId, currentViewMonday, 1);
    view->updateTable(approvedShifts);

    // ── Pending and Declined shifts (top table) ──────────────────────────────
    QMap<int, QList<Shift*>> pendingShifts = model->getRawStaffShifts(currentEmployeeId, currentViewMonday, 0);
    QMap<int, QList<Shift*>> declinedShifts = model->getRawStaffShifts(currentEmployeeId, currentViewMonday, -1);
    
    QMap<int, QList<Shift*>> pendingAndDeclined;
    for (int col = 0; col < 7; ++col) {
        if (pendingShifts.contains(col)) {
            pendingAndDeclined[col].append(pendingShifts[col]);
        }
        if (declinedShifts.contains(col)) {
            pendingAndDeclined[col].append(declinedShifts[col]);
        }
    }
    view->updatePendingTable(pendingAndDeclined);

    // Clean up temporary shift objects
    for (auto list : approvedShifts) qDeleteAll(list);
    for (auto list : pendingShifts) qDeleteAll(list);
    for (auto list : declinedShifts) qDeleteAll(list);

    updateDateRangeLabel();

    QDate today = QDate::currentDate();
    if (today >= currentViewMonday && today <= currentViewMonday.addDays(6)) {
        view->highlightToday(currentViewMonday.daysTo(today));
    } else {
        view->highlightToday(-1);
    }
}


void ViewSchedule_Control::loadManagerSchedule() {
    view->setManagerFeaturesVisible(true);
    
    // Cleanup old grid
    for (int col = 0; col < 7; ++col) {
        if (scheduleGrid.contains(col)) {
            qDeleteAll(scheduleGrid[col]);
        }
    }
    scheduleGrid.clear();

    scheduleGrid = model->getManagerWeeklyGrid(currentViewMonday);

    view->updateManagerTable(scheduleGrid);
    updateDateRangeLabel();
    view->highlightToday(-1); 
}

void ViewSchedule_Control::onShiftClicked(int row, int dayIndex) {
    if ((dayIndex < 0) || (dayIndex > 6)) return;
    if (!scheduleGrid.contains(dayIndex) || !scheduleGrid[dayIndex].contains(row)) return;
    
    ShiftBlock* block = scheduleGrid[dayIndex][row];

    if (!block->isEmpty()) {
        view->updateShiftDetails(block->getEmployees(), block->getShiftIds(), block->getTimeString());

    } else {
        view->updateShiftDetails(QList<User*>(), QList<int>(), "");
    }
}

void ViewSchedule_Control::onShowReplacementsRequested(int oldShiftId, const QString &role)
{
    if (!model || !view) return;
    
    QList<PendingShiftInfo> replacements = model->getEligibleReplacements(oldShiftId, role);
    view->showReplacementDialog(oldShiftId, replacements);
}

void ViewSchedule_Control::onConfirmReplacement(int oldShiftId, int newShiftId)
{
    if (!model || !view) return;
    
    if (model->replaceShift(oldShiftId, newShiftId)) {
        // Refresh grid
        loadManagerSchedule();
    }
}



void ViewSchedule_Control::updateDateRangeLabel()
{
    view->updateDateRange(currentViewMonday);
}

void ViewSchedule_Control::onPrevWeek() {
    currentViewMonday = currentViewMonday.addDays(-7);
    loadData();
}
void ViewSchedule_Control::onNextWeek() {
    currentViewMonday = currentViewMonday.addDays(7);
    loadData();
}
void ViewSchedule_Control::onCurrentWeek() { load(); }