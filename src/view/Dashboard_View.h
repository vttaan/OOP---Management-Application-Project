#ifndef DASHBOARD_VIEW_H
#define DASHBOARD_VIEW_H
#include <QEvent>
#include <QWidget>
#include <QHBoxLayout>
#include "control/Dashboard_Control.h"

class Dashboard_Control;

namespace Ui {
class Dashboard_View;
}

class Dashboard_View : public QWidget
{
    Q_OBJECT

public:
    Dashboard_View(Dashboard_Control* controller = nullptr, QWidget *parent = nullptr);
    Dashboard_Control* getController() const;
    ~Dashboard_View();
<<<<<<< Updated upstream
=======
    Dashboard_Control *getController() const;
    void setController(Dashboard_Control *controller);

>>>>>>> Stashed changes
protected:
    bool eventFilter(QObject*watched,QEvent*event)override;

private slots:
    void on_btnLogout_clicked();
private:
    Ui::Dashboard_View *ui;
    Dashboard_Control* controller;
signals:
};



class clickableBox : public QWidget {
    Q_OBJECT
private:
    QHBoxLayout* layout;
public:
    clickableBox(QWidget* parent = nullptr) : QWidget(parent) {
        layout = new QHBoxLayout(this);
    }
    QHBoxLayout* getLayout() { return layout; }
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent* event) override {
        emit clicked();
    }
};


#endif
