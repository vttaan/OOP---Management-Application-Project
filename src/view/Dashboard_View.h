#ifndef DASHBOARD_VIEW_H
#define DASHBOARD_VIEW_H

#include <QWidget>
#include <QMouseEvent>
#include <QHBoxLayout>

class Dashboard_Control;
class EmployeeCard;

namespace Ui {
class Dashboard_View;
}

class Dashboard_View : public QWidget
{
    Q_OBJECT

public:
    // Khai báo chuẩn khớp với kiến trúc nhóm
    explicit Dashboard_View(Dashboard_Control *controller = nullptr, QWidget *parent = nullptr);
    ~Dashboard_View();
    void updateStatCards(int total, int staff, int manager, int workingToday);
    void clearEmployeeGrid();
    void addEmployeeCard(EmployeeCard* card);
    void updateShiftPanel(const QList<QPair<QString,QString>>& nextShifts,const QStringList& absentNames);
signals:
    void profileClicked();
    void searchChanged(const QString&text);


protected:
    //bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::Dashboard_View *ui;
    Dashboard_Control *controller;
};

#endif // DASHBOARD_VIEW_H