#ifndef DASHBOARD_VIEW_H
#define DASHBOARD_VIEW_H

#include <QWidget>
#include "model/Dashboard_Model.h"   // for ShiftEmployeeInfo DTO
#include "utils/NotificationDTOs.h"

class Dashboard_Control;
class EmployeeCard;

namespace Ui {
class Dashboard_View;
}

class Dashboard_View : public QWidget
{
    Q_OBJECT

public:
    explicit Dashboard_View(QWidget *parent = nullptr);
    ~Dashboard_View();

    void clearEmployeeGrid();
    void addEmployeeCard(EmployeeCard* card);

    // Panel 2 - takes structured DTOs directly from Model (via Controller)
    void updateNextShiftPanel(const QList<ShiftEmployeeInfo>& entries);

    void updateLeaveRequestPanel(const QList<LeaveRequestInfo>& requests);
    void setLeaveRequestPanelVisible(bool visible);

    // Panel 3 - chart with VND tooltip support
    void updateSalaryChart(const QVector<double>& lastYear, const QVector<double>& thisYear,
                           int lastYearEmpCount, int thisYearEmpCount,
                           int selectedYear);
    void setSalaryChartVisible(bool visible);

signals:
    void profileClicked();
    void yearChanged(int year);   // Emitted when user clicks a different year tab
    void leaveRequestReviewRequested(int requestId, bool approved);

private slots:
    void onYearTabClicked(int index);
    void onBarHovered(bool status, int index, QBarSet* barSet);   // Tooltip on hover

private:
    Ui::Dashboard_View *ui;

    QVBoxLayout* m_nextShiftLayout  = nullptr;
    QVBoxLayout* m_leaveRequestLayout = nullptr;
    QLabel*      m_lblLastYearCount = nullptr;
    QLabel*      m_lblThisYearCount = nullptr;
    QLabel*      m_tooltipLabel     = nullptr;   // Hover tooltip for chart bars
    QChart*      m_chart            = nullptr;
    QChartView*  m_chartView        = nullptr;
    QFrame*      m_salaryCard       = nullptr;
    QFrame*      m_leaveRequestCard = nullptr;
    QWidget*     m_empGridWidget    = nullptr;
    QTabBar*     m_yearTabBar       = nullptr;

    QVector<int> m_availableYears;
    QFrame* makeCard(const QString& title, QLayout* innerLayout, bool isDark = false);
};

#endif // DASHBOARD_VIEW_H
