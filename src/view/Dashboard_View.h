#include "global.h"
#ifndef DASHBOARD_VIEW_H
#define DASHBOARD_VIEW_H

#include <QWidget>

namespace Ui {
class Dashboard_View;
}

class Dashboard_View : public QWidget
{
    Q_OBJECT

public:
    explicit Dashboard_View(QWidget *parent = nullptr);
    ~Dashboard_View();

signals:

    void profileClicked();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::Dashboard_View *ui;
};

#endif