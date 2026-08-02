#include "global.h"
#include "core/ShiftBlock.h"
#include "utils/ScheduleDTOs.h"
#include "utils/Config.h"
#include "core/Shift.h"
#ifndef SCHEDULE_VIEW_H
#define SCHEDULE_VIEW_H

namespace Ui {
class Schedule_View;
}

// Each entry: { date string, shift name, required count, assigned count }
struct MissingShiftInfo {
    QString dateStr;
    QString shiftName;
    int required = 0;
    int assigned = 0;
};


class Schedule_View : public QWidget
{
    Q_OBJECT

public:
    explicit Schedule_View(QWidget *parent = nullptr);
    ~Schedule_View();

    // for allow registrate, only fixed day to registrate
    void enableRegistration(bool isEnable);

    // show error when time overlapping
    void showError(const QString& mess);

    // Populate the interactive grid with date-labelled column headers
    void setUpInteractiveGrid(QDate weekStart, int openTime, int closeTime);

    // Populate the full-time 3-shift registration grid from controller-owned state.
    void setUpFullTimeGrid(QDate weekStart, const FullTimeScheduleGrid &statuses);
    void showFullTimeSaveFeedback(const QString &message);

    // Update column headers of the summary (manager) table
    void updateTableHeaders(QDate monday);

    // Color the interactive grid for staff based on shift status
    void updateStaffInteractiveGridStatus(
        const QMap<int, QList<Shift*>> &pendingShifts,
        const QMap<int, QList<Shift*>> &acceptedShifts,
        const QMap<int, QMap<int, ShiftBlock *>> &managerGrid);
    void showSuccess(const QString& msg);
    void showWarnings(const QStringList& warnings);

    void setManagerMode(bool isManager);

    // ── Xem Lich Lam (View) grid — accepted shifts only ──────────────────────
    void updateManagerPendingGrid(const QMap<int, QMap<int, ShiftBlock*>>& grid);

    // -- Xep Lich Lam (Assign) grid -- shows pending/accepted/declined counts -
    void updateAssignGrid(const QMap<int, QMap<int, BlockCounts>>& counts);

    // Missing staff table (manager only, shown below the grid in Xep lich page)
    void updateManagerMissingShifts(const QList<MissingShiftInfo>& missingList);

    // Opens a popup dialog showing the requests for a given shift block.
    // The dialog emits approve/decline signals back to the controller.
    void showShiftRequestsDialog(const QList<PendingShiftInfo>& requests,
                                 const QString& shiftLabel);

private:
    Ui::Schedule_View *ui;
    void setUpUI();

    // Build the staff-mode interactive calendar grid
    void buildInteractiveGrid(int openHour, int closeHour);
    void buildFullTimeGrid();
    void renderFullTimeCell(int row, int col);
    void updateFullTimeWeekMetadata(QDate weekStart);
    void resetFullTimeFooterHint();
    bool eventFilter(QObject *watched, QEvent *event) override;

    // Manager missing-staff table (created dynamically)
    QWidget*        missingStaffWidget;
    QLabel*         lblMissingStaffHeader;
    QLabel*         lblMissingCount;
    QTableWidget*   tableMissingStaff;

    QWidget*        fullTimeInfoWidget;
    QLabel*         lblFullTimeWeekRange;
    QLabel*         lblFullTimeFooterMessage;

    // Tracks which mode the manager grid is in (assign vs view)
    int             m_isAssignMode = -1;

    // Store open/close hours used to build the interactive grid
    int             m_openHour  = Config::getOpenHour();
    int             m_closeHour = Config::getCloseHour();
    bool            m_isFullTimeMode = false;
    FullTimeScheduleGrid m_fullTimeStatuses;
    QSet<QPair<int, int>> m_fullTimeSelections;

signals:

    // Staff registration: emitted with grid selection when Luu is clicked
    // Outer QList index = day (0-6), inner QList = selected hour indices (row indices)
    void requestSaveGridShifts(const QList<QList<int>>& selectedHoursByDay);

    // Full-time mock registration: selected shift-row indices grouped by day.
    void requestSaveFullTimeShifts(const QList<QList<int>>& selectedShiftsByDay);

    // switch tab
    void profileClicked();
    void requestGenSchedule();
    void requestConfirm();

    // Fired when manager clicks a shift block in assign mode
    void shiftBlockClicked(int col, int row);

    // Fired from the popup dialog
    void requestApproveShift(int shiftId);
    void requestDeclineShift(int shiftId);

private slots:
    void buttonSaveClicked();
    void onFullTimeCellClicked(int row, int col);
};

#endif // SCHEDULE_VIEW_H
