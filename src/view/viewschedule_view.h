#ifndef VIEWSCHEDULE_VIEW_H
#define VIEWSCHEDULE_VIEW_H
#include "global.h"
namespace Ui {
class ViewSchedule_View;
}
class QSplitter;
class ShiftBlock;
class User;

namespace ScheduleStyle {
    const QString BtnNormal = 
        "QPushButton { background-color: #F3F4F6; color: #374151; border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 15px; font-weight: bold; } "
        "QPushButton:hover { background-color: #E5E7EB; }";

    const QString BtnHighlight = 
        "QPushButton { background-color: #2F80ED; color: white; border-radius: 6px; padding: 6px 15px; font-weight: bold; } "
        "QPushButton:hover { background-color: #1C64F2; }";

    const QString Title = "font-size: 14px; font-weight: bold; color: black; padding-bottom: 5px;";

    inline QString getTableStyle(const QString& headerBgColor) {
        return QString("QTableWidget { border: 1px solid #E5E7EB; background-color: white; gridline-color: #E5E7EB; } "
                       "QHeaderView::section { background-color: %1; border: none; border-right: 1px solid #E5E7EB; border-bottom: 2px solid #E5E7EB; padding: 8px; font-weight: bold; color: black; text-transform: uppercase; }").arg(headerBgColor);
    }

    inline QString getVerticalTableStyle(const QString& headerBgColor) {
        return QString("QTableWidget { border: none; background-color: white; gridline-color: #EFEFEF; } "
                       "QHeaderView::section { background-color: %1; border: none; border-bottom: 2px solid #E5E7EB; border-right: 2px solid #E5E7EB; padding: 8px; font-weight: bold; color: black; text-transform: uppercase; } ").arg(headerBgColor);
    }

    // Color Constants
    const QString ColorStaffHeader = "#E0F2FE";       // Light Blue
    const QString ColorManagerHeader = "#F3E8FF";     // Pastel Purple
    const QString ColorManagerBottomHeader = "#E9D5FF"; // Darker Pastel Purple (10% darker)
}

class ViewSchedule_View : public QWidget
{
    Q_OBJECT
public:
    explicit ViewSchedule_View(QWidget *parent = nullptr);
    ~ViewSchedule_View();
    void updateTable(const QMap<int, QList<QString>>& weeklyData);
    void updateManagerTable(const QList<QString>& timeSlots, const QMap<int, QMap<int, ShiftBlock*>>& gridData);
    void updateDateRange(const QString& dateRangeText);
    void highlightToday(int currentDayIndex);
    
    // New methods for bottom pane
    void updateUnqualifiedShifts(const QList<QPair<QString, int>>& unqualifiedShifts);
    void updateShiftDetails(const QList<User*>& employees, const QString& timeLabel);
    void setManagerFeaturesVisible(bool visible);
    
signals:
    void requestPrevWeek();
    void requestNextWeek();
    void requestCurrentWeek();
    void shiftClicked(int row, int dayIndex);
    
private:
    Ui::ViewSchedule_View *ui;
    void setUpUI();
    
    QSplitter* splitter;
    QWidget* bottomWidget;
    QTableWidget* tableMissingStaff;
    QTableWidget* tableShiftDetails;
    
private slots:
    void onBtnPrevClicked();
    void onBtnNextClicked();
    void onBtnCurrentClicked();

};
#endif // VIEWSCHEDULE_VIEW_H