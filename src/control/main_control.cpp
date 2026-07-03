#include "main_control.h"
#include <QDebug>

Main_Control::Main_Control(QObject *parent) : QObject(parent), view(new Main_View())
{
    connect(view, &Main_View::menuOverviewClicked, this, &Main_Control::handleOverviewClicked);
    connect(view, &Main_View::menuHRClicked, this, &Main_Control::handleHRClicked);
    connect(view, &Main_View::menuTimekeepClicked, this, &Main_Control::handleTimekeepClicked);
    connect(view, &Main_View::menuSalaryClicked, this, &Main_Control::handleSalaryClicked);
    connect(view, &Main_View::menuReportClicked, this, &Main_Control::handleReportClicked);
    connect(view, &Main_View::menuSettingsClicked, this, &Main_Control::handleSettingsClicked);
    connect(view, &Main_View::profileClicked, this, &Main_Control::handleProfileClicked);
}

Main_Control::~Main_Control()
{
    if (view) delete view;
}

void Main_Control::init()
{
    view->show();
}

void Main_Control::handleOverviewClicked() {
    qDebug() << "Mo trang Tong Quan";
    view->switchPage(0);
}

void Main_Control::handleHRClicked() {
    qDebug() << "Mo trang Nhan Su";
    view->switchPage(1);
}

void Main_Control::handleTimekeepClicked() {
    view->switchPage(2);
}

void Main_Control::handleSalaryClicked() {
    view->switchPage(3);
}

void Main_Control::handleReportClicked() {
    view->switchPage(4);
}

void Main_Control::handleSettingsClicked() {
    view->switchPage(5);
}

void Main_Control::handleProfileClicked() {
    view->switchPage(6);
}