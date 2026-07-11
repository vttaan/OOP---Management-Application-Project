#ifndef DASHBOARD_VIEW_H
#define DASHBOARD_VIEW_H

#include <QWidget>

class Dashboard_Control;

namespace Ui {
class Dashboard_View;
}

class Dashboard_View : public QWidget
{
    Q_OBJECT

public:
    explicit Dashboard_View(Dashboard_Control *controller = nullptr, QWidget *parent = nullptr);
    ~Dashboard_View();

    Dashboard_Control *getController() const;
    void setController(Dashboard_Control *controller);

    // Các hàm quản lý khung sườn
    void addSidebar(QWidget* sidebarWidget);
    void embedWidgetIntoPage(int index, QWidget* widget);

public slots:
    void switchPage(int index);
    void toggleSidebar();

private:
    Ui::Dashboard_View *ui;
    Dashboard_Control *controller;
    QWidget* currentSidebar;      // Biến lưu trữ thanh Sidebar
};

#endif // DASHBOARD_VIEW_H