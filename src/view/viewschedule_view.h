#ifndef VIEWSCHEDULE_VIEW_H
#define VIEWSCHEDULE_VIEW_H
#include "global.h"
#include "utils/ScheduleDTOs.h"
namespace Ui
{
    class ViewSchedule_View;
}
class ShiftBlock;
class User;
class Shift;

namespace ScheduleStyle
{
    // Blue-White theme aligned with navigation bar
    const QString BtnNormal =
        "QPushButton { background-color: #F3F4F6; color: #374151; border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 15px; font-weight: bold; } "
        "QPushButton:hover { background-color: #E5E7EB; }";

    const QString BtnHighlight =
        "QPushButton { background-color: #2F80ED; color: white; border-radius: 6px; padding: 6px 15px; font-weight: bold; } "
        "QPushButton:hover { background-color: #1C64F2; }";

    const QString Title = "font-size: 14px; font-weight: bold; color: #1F2937; padding-bottom: 5px;";

    // 3 canonical shifts: Morning, Afternoon, Evening
    const QStringList SHIFT_NAMES = {"Ca Sáng", "Ca Chiều", "Ca Tối"};
}

class ViewSchedule_View : public QWidget
{
    Q_OBJECT
public:
    explicit ViewSchedule_View(QWidget *parent = nullptr);
    ~ViewSchedule_View();

    // Staff view: display assigned (approved) shifts per day
    void updateTable(const QMap<int, QList<Shift*>> &weeklyData);

    // Staff view: display pending (unreviewed) and declined shifts per day
    void updatePendingTable(const QMap<int, QList<Shift*>> &weeklyData);

    // Manager view: display finalized schedule with employee cards
    // grid: col (day 0-6) -> row (shift 0-2) -> ShiftBlock*
    void updateManagerTable(const QMap<int, QMap<int, ShiftBlock *>> &gridData);

    void updateDateRange(QDate monday);
    void highlightToday(int currentDayIndex);

    void updateShiftDetails(const QList<User *> &employees, const QList<int>& shiftIds, const QString &timeLabel);
    void updateShiftDetails(const QList<User *> &employees, const QList<int>& shiftIds, const QString &timeLabel, const QMap<int, QString> &employeeTimes);

    void setManagerFeaturesVisible(bool visible);
    
    void showReplacementDialog(int oldShiftId, const QList<PendingShiftInfo>& replacements);

signals:
    void requestPrevWeek();
    void requestNextWeek();
    void requestCurrentWeek();
    void shiftClicked(int row, int dayIndex);
    
    void requestShowReplacements(int shiftId, const QString &role);
    void requestConfirmReplacement(int oldShiftId, int newShiftId);

private:
    Ui::ViewSchedule_View *ui;
    void setUpUI();
    QWidget *detailsWidget;
    QLabel *lblShiftDetailTitle;
    QLabel *lblShiftDetailSubtitle;
    QLabel *lblShiftDetailCount;
    QTableWidget *tableShiftDetails;
    QComboBox *managerViewMode = nullptr;
    QComboBox *managerRoleFilter = nullptr;
    QMap<int, QMap<int, ShiftBlock *>> m_managerGrid;
    bool m_isManagerMode = false; // guard for redundant polish calls

private slots:
    void onBtnPrevClicked();
    void onBtnNextClicked();
    void onBtnCurrentClicked();
};
#endif // VIEWSCHEDULE_VIEW_H
