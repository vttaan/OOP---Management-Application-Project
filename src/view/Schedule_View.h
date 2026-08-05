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
    int dayColumn = -1;
    int shiftRow = -1;
};


class Schedule_View : public QWidget
{
    Q_OBJECT

public:
    explicit Schedule_View(QWidget *parent = nullptr);
    ~Schedule_View();

    // Staff scheduling API (implemented in Schedule_View_Staff.cpp)
    // for allow registrate, only fixed day to registrate
    void enableRegistration(bool isEnable);
    void setPartTimeRegistrationState(bool isOpen, QDate nextOpenDate);

    // show error when time overlapping
    void showError(const QString& mess);

    // Populate the interactive grid with date-labelled column headers
    void setUpInteractiveGrid(QDate weekStart, int openTime, int closeTime);

    // Populate the database-backed full-time 3-shift registration grid.
    void setUpFullTimeScheduleGrid(QDate weekStart,
                                   const FullTimeScheduleGrid &statuses);
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

    // Manager scheduling API (implemented in Schedule_View_Manager.cpp)
    void setManagerMode(bool isManager);

    // ── Xem Lich Lam (View) grid — accepted shifts only ──────────────────────
    void updateManagerPendingGrid(const QMap<int, QMap<int, ShiftBlock*>>& grid);

    // -- Xep Lich Lam (Assign) grid -- shows pending/accepted/declined counts -
    void updateAssignGrid(const QMap<int, QMap<int, BlockCounts>>& counts);

    // Missing staff table (manager only, shown below the grid in Xep lich page)
    void updateManagerMissingShifts(const QList<MissingShiftInfo>& missingList);
    void setManagerDraftStatus(int changeCount);
    void resetManagerAddButton();
    void updateManagerSummary(int totalShifts, int shortageShifts,
                              int pendingRequests, int draftChanges,
                              int missingSlots = 0, int staffedShifts = 0);
    void updateManagerWeek(QDate monday);
    void selectManagerShift(int dayColumn, int shiftRow);

    // Opens a popup dialog showing the requests for a given shift block.
    // The dialog emits approve/decline signals back to the controller.
    void showShiftRequestsDialog(const QList<PendingShiftInfo>& requests,
                                 const QString& shiftLabel,
                                 const QList<EligibleEmployeeInfo>& eligibleEmployees = {},
                                 QDate shiftDate = {}, QTime blockStart = {},
                                 QTime blockEnd = {});

private:
    Ui::Schedule_View *ui;
    // Shared widget construction and mode-independent styling.
    void setUpUI();

    // Staff-only helpers (implemented in Schedule_View_Staff.cpp)
    void buildInteractiveGrid(int openHour, int closeHour);
    void buildFullTimeGrid();
    void renderFullTimeCell(int row, int col);
    void updateFullTimeWeekMetadata(QDate weekStart);
    void resetFullTimeFooterHint();
    void updatePartTimeWeekMetadata(QDate weekStart);
    void updatePartTimeInfoText();
    void setPartTimeItemSelected(QTableWidgetItem *item, bool selected);
    bool eventFilter(QObject *watched, QEvent *event) override;

    // Manager-only widgets and state (used by Schedule_View_Manager.cpp)
    QWidget*        missingStaffWidget;
    QLabel*         lblMissingStaffHeader;
    QLabel*         lblMissingCount;
    QTableWidget*   tableMissingStaff;
    QLabel*         lblManagerDraftStatus;
    QLabel*         lblManagerSummary;
    QLabel*         lblManagerTotal = nullptr;
    QLabel*         lblManagerShortage = nullptr;
    QLabel*         lblManagerPending = nullptr;
    QLabel*         lblManagerDraft = nullptr;
    QLabel*         lblManagerWeek = nullptr;
    QComboBox*      managerStatusFilter = nullptr;
    QComboBox*      managerRoleFilter = nullptr;
    QPushButton*    managerUndoDraftButton = nullptr;
    QPushButton*    managerClearDraftButton = nullptr;
    QPushButton*    requestLeaveButton = nullptr;
    QPushButton*    leaveHistoryButton = nullptr;
    QFrame*         shiftDetailDrawer = nullptr;
    QVBoxLayout*    shiftDetailDrawerLayout = nullptr;
    QPushButton*    activeManagerAddButton = nullptr;
    bool            managerAddRejectedDuringRequest = false;

    QWidget*        fullTimeInfoWidget;
    QLabel*         lblFullTimeWeekRange;
    QLabel*         lblFullTimeFooterMessage;
    QStackedWidget* staffInfoStack;
    QWidget*        partTimeInfoWidget;
    QLabel*         lblPartTimeRegistrationState;
    QLabel*         lblPartTimeWeekRange;
    QLabel*         lblPartTimeFooterMessage;

    // Tracks which mode the manager grid is in (assign vs view)
    int             m_isAssignMode = -1;

    // Store open/close hours used to build the interactive grid
    int             m_openHour  = Config::getOpenHour();
    int             m_closeHour = Config::getCloseHour();
    bool            m_isFullTimeMode = false;
    bool            m_partTimeRegistrationOpen = false;
    bool            m_partTimeDragActive = false;
    bool            m_partTimeDragSelect = true;
    QDate           m_partTimeNextOpenDate;
    QSet<QPair<int, int>> m_partTimeDragVisited;
    QSet<QPair<int, int>> m_partTimePendingCells;
    QSet<int>       m_partTimeApprovedDays;
    FullTimeScheduleGrid m_fullTimeStatuses;
    QSet<QPair<int, int>> m_fullTimeSelections;
    QMap<int, QMap<int, BlockCounts>> m_lastAssignCounts;
    int m_selectedManagerDay = -1;
    int m_selectedManagerShift = -1;
    QList<PendingShiftInfo> m_lastDrawerRequests;
    QList<EligibleEmployeeInfo> m_lastDrawerEligible;
    QString m_lastDrawerShiftLabel;
    QDate m_lastDrawerDate;
    QTime m_lastDrawerStart;
    QTime m_lastDrawerEnd;
    QList<ManagerEmployeeSelection> m_managerEmployeeSelections;
    QDate m_managerSelectionDate;
    QTime m_managerSelectionStart;
    QTime m_managerSelectionEnd;

signals:

    // Staff registration: emitted with grid selection when Luu is clicked
    // Outer QList index = day (0-6), inner QList = selected hour indices (row indices)
    void requestSaveGridShifts(const QList<QList<int>>& selectedHoursByDay);

    // Full-time registration: selected pending shift rows grouped by day.
    void requestSaveFullTimeSchedule(const QList<QList<int>>& selectedShiftsByDay);

    // switch tab
    void profileClicked();
    void requestGenSchedule();
    void requestConfirm();

    // Fired when manager clicks a shift block in assign mode
    void shiftBlockClicked(int col, int row);

    // Fired from the popup dialog
    void requestApproveShift(PendingShiftInfo request);
    void requestDeclineShift(PendingShiftInfo request);
    void requestAddEmployees(QDate date, QTime blockStart, QTime blockEnd,
                             const QList<ManagerEmployeeSelection> &selections);
    void requestRemoveAssignedShift(int shiftId, int employeeId,
                                    const QString &reason);
    void requestPreviousManagerWeek();
    void requestNextManagerWeek();
    void requestCurrentManagerWeek();
    void requestUndoManagerDraft();
    void requestClearManagerDraft();
    void requestLeave();
    void requestLeaveHistory();

private slots:
    void buttonSaveClicked();
    void clearPendingSelections();
    void onFullTimeCellClicked(int row, int col);
};

#endif // SCHEDULE_VIEW_H
