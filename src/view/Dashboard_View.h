#ifndef DASHBOARD_VIEW_H
#define DASHBOARD_VIEW_H

#include <QWidget>
#include <QMouseEvent>
#include <QHBoxLayout>

class Dashboard_Control;

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

signals:
    void profileClicked();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::Dashboard_View *ui;
    Dashboard_Control *controller;
};

#endif // DASHBOARD_VIEW_H