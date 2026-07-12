#include "Dashboard_View.h"
#include "ui_Dashboard_View.h"
#include "control/Dashboard_Control.h"

Dashboard_View::Dashboard_View(Dashboard_Control *controller, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Dashboard_View),
    controller(controller)
{
    ui->setupUi(this);

    if (ui->lblAvatar) {
        ui->lblAvatar->setCursor(Qt::PointingHandCursor);
        ui->lblAvatar->installEventFilter(this);
    }
    if (ui->lblUserName) {
        ui->lblUserName->setCursor(Qt::PointingHandCursor);
        ui->lblUserName->installEventFilter(this);
    }
}

Dashboard_View::~Dashboard_View()
{
    delete ui;
}

bool Dashboard_View::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        if (watched == ui->lblAvatar || watched == ui->lblUserName)
        {
            emit profileClicked();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}