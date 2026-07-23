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
}

void ViewSchedule_Control::load() {
    if (!view || currentEmployeeId < 0) return;

    currentViewMonday = QDate::currentDate().addDays(-(QDate::currentDate().dayOfWeek() - 1));
    loadData();
}


void ViewSchedule_Control::loadData() {
    if (!this->currentSession || !this->currentSession->getCurrentUser()) {
        qDebug() << "ViewSchedule_Control::loadData() - No session or user found";
        return;
    }
    
    QString currentRole = this->currentSession->getCurrentUser()->getRole();
    
    if (currentRole == "Manage" || currentRole == "Admin") {
        loadManagerSchedule();
    } else {
        loadStaffSchedule();
    }
}

void ViewSchedule_Control::loadStaffSchedule() {
    model->getAcceptedSchedule(currentEmployeeId, currentViewMonday);
    view->setManagerFeaturesVisible(false);
    view->updateTable(model->getWeeklySummaryStrings());
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

    QList<QString> timeSlots;
    for (int i = 8; i < 22; ++i) {
        QString start = QString("%1:00").arg(i, 2, 10, QChar('0'));
        QString end = QString("%1:00").arg(i+1, 2, 10, QChar('0'));
        timeSlots.append(start + " - " + end);
    }

    QList<QString> missingShifts;
    QStringList daysOfWeek = {"Thứ 2", "Thứ 3", "Thứ 4", "Thứ 5", "Thứ 6", "Thứ 7", "Chủ Nhật"};

    for (int col = 0; col < 7; ++col) {
        if (!scheduleGrid.contains(col)) continue;
        for (int row = 0; row < timeSlots.size(); ++row) {
            if (!scheduleGrid[col].contains(row)) continue;
            ShiftBlock* block = scheduleGrid[col][row];
            if (block && (block->getStatus() == ShiftStatus::Understaffed || block->getStatus() == ShiftStatus::Empty)) {
                QString shiftName = QString("%1: %2").arg(daysOfWeek[col]).arg(timeSlots[row]);
                QString countStr = QString::number(block->getStaffCount());
                missingShifts.append(shiftName + "|" + countStr);
            }
        }
    }

    view->updateManagerTable(timeSlots, scheduleGrid);
    view->updateMissingStaff(missingShifts);
    updateDateRangeLabel();
    view->highlightToday(-1); 
}

void ViewSchedule_Control::onShiftClicked(int row, int dayIndex) {
    if ((dayIndex < 0) || (dayIndex > 6)) return;
    if (!scheduleGrid.contains(dayIndex) || !scheduleGrid[dayIndex].contains(row)) return;
    
    ShiftBlock* block = scheduleGrid[dayIndex][row];

    if (!block->isEmpty()) {
        view->updateShiftDetails(block->getEmployees(), block->getTimeString());
    }
}



void ViewSchedule_Control::updateDateRangeLabel() {
    QString start = currentViewMonday.toString("dd/MM/yyyy");
    QString end = currentViewMonday.addDays(6).toString("dd/MM/yyyy");
    view->updateDateRange(start + " - " + end);
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