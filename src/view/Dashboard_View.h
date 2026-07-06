#ifndef DASHBOARD_VIEW_H
#define DASHBOARD_VIEW_H

#include <QEvent>
#include <QWidget>
#include <QHBoxLayout>
#include "control/Control_Navigator.h"

class Dashboard_Control;

namespace Ui
{
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

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_btnMenu_Overview_clicked();
    void on_btnMenu_HR_clicked();
    void on_btnMenu_Timekeep_clicked();
    void on_btnMenu_Salary_clicked();
    void on_btnMenu_Report_clicked();
    void on_btnMenu_Settings_clicked();
    void on_btnLogout_clicked();

private:
    Ui::Dashboard_View *ui;
    Dashboard_Control *controller;
signals:
    // View forwards user actions to its controller by calling controller signals directly
};

class clickableBox : public QWidget
{
    Q_OBJECT
private:
    QHBoxLayout *layout;

public:
    clickableBox(QWidget *parent = nullptr) : QWidget(parent)
    {
        layout = new QHBoxLayout(this);
    }
    QHBoxLayout *getLayout() { return layout; }
signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        emit clicked();
    }
};

#endif