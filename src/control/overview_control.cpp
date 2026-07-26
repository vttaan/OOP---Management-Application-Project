#include "Overview_Control.h"
#include "view/Overview_view.h"

Overview_Control::Overview_Control(QObject *parent)
    : QObject(parent), view(new Overview_View())
{
}

Overview_Control::~Overview_Control()
{
    if (view) delete view;
}

void Overview_Control::init()
{
    if (!view) return;

    view->clearEmployeeCards();
    view->addEmployeeCard(":/images/image1.jpg", "Nhân viên A", "Nhân viên", "a@congty.com", "0901 111 111");
    view->addEmployeeCard(":/images/image2.jpg", "Nhân viên B", "Nhân viên", "b@congty.com", "0902 222 222");
    view->addEmployeeCard(":/images/image3.jpg", "Nhân viên C", "Nhân viên", "c@congty.com", "0903 333 333");
    view->addEmployeeCard(":/images/image4.jpg", "Nhân viên D", "Nhân viên", "d@congty.com", "0904 444 444");
    view->addEmployeeCard(":/images/image5.jpg", "Nhân viên E", "Nhân viên", "e@congty.com", "0905 555 555");
    view->addEmployeeCard(":/images/image6.jpg", "Quản lý 1", "Quản lý", "ql1@congty.com", "0906 666 666");
    view->addEmployeeCard(":/images/image7.jpg", "Quản lý 2", "Quản lý", "ql2@congty.com", "0907 777 777");

    view->clearSidePanels();
    view->addNextShiftItem("TÊN C", "15H-23H", "#d9d9d9");
    view->addNextShiftItem("TÊN D", "07H-15H", "#d9d9d9");
    view->addNextShiftItem("TÊN E", "23H-07H", "#d9d9d9");

    view->addOffEmployeeItem("TÊN F", "Nghỉ ốm", "#ff4d4f");
    view->addOffEmployeeItem("TÊN G", "Nghỉ phép", "#ff4d4f");
}

Overview_View* Overview_Control::getView() const
{
    return view;
}