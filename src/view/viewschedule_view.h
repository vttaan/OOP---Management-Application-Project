#ifndef VIEWSCHEDULE_VIEW_H
#define VIEWSCHEDULE_VIEW_H
#include "global.h"
namespace Ui {
class ViewSchedule_View;
}
class QSplitter;
class ShiftBlock;
class User;

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
    void updateMissingStaff(const QList<QString>& missingShifts);
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