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

    view->clearEmployeeCards();

    view->addEmployeeCard(":/images/image1.jpg", "Nhân viên A", "Staff", "a@congty.com", "0901 111 111");
    view->addEmployeeCard(":/images/image2.jpg", "Nhân viên B", "Staff", "b@congty.com", "0902 222 222");
    view->addEmployeeCard(":/images/image1.jpg", "Nhân viên C", "Staff", "c@congty.com", "0903 333 333");
    view->addEmployeeCard(":/images/image2.jpg", "Nhân viên D", "Manager", "d@congty.com", "0904 444 444");
    view->addEmployeeCard(":/images/image1.jpg", "Nhân viên E", "Staff", "e@congty.com", "0905 555 555");
    view->addEmployeeCard(":/images/image2.jpg", "Nhân viên F", "Staff", "f@congty.com", "0906 666 666");
    view->addEmployeeCard(":/images/image1.jpg", "Nhân viên G", "Staff", "g@congty.com", "0907 777 777");
    view->clearSidePanels();

    view->addNextShiftItem("TÊN C", "15H-23H", "#d9d9d9");
    view->addNextShiftItem("TÊN D", "07H-15H", "#d9d9d9");
    view->addNextShiftItem("TÊN E", "23H-07H", "#d9d9d9");

    view->addOffEmployeeItem("TÊN F", "Nghỉ ốm", "#ff4d4f");
    view->addOffEmployeeItem("TÊN G", "Nghỉ phép", "#ff4d4f");
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
    qDebug() << "Mo trang Cham Cong";
    view->switchPage(2);
}

void Main_Control::handleSalaryClicked() {
    qDebug() << "Mo trang Luong";
    view->switchPage(3);

}

void Main_Control::handleReportClicked() {
    qDebug() << "Mo trang Bao Cao";
    view->switchPage(4);
}

void Main_Control::handleSettingsClicked() {
    qDebug() << "Mo trang Cai Dat";
    view->switchPage(5);
}

void Main_Control::handleProfileClicked() {
    qDebug() << "Mo trang Quan Tri Vien";
    view->switchPage(6);
}