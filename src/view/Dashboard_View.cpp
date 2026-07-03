#include "Dashboard_View.h"
#include "ui_Dashboard_View.h"
#include "control/Dashboard_Control.h"
#include "employeecard.h"
#include "employeeswidget.h"
#include <QGridLayout>
#include <QMouseEvent>

Dashboard_View::Dashboard_View(Dashboard_Control *controller, QWidget *parent) : QWidget(parent),
                                                                                 ui(new Ui::Dashboard_View()),
                                                                                 controller(controller)
{
    ui->setupUi(this);

    // Create a grid of employee cards inside the scroll area
    QGridLayout *gridLayout = new QGridLayout(ui->scrollAreaWidgetContents);
    gridLayout->setSpacing(20);

    QStringList avatars = {":/images/image1.jpg", ":/images/image2.jpg", ":/images/image3.jpg",
                           ":/images/image4.jpg", ":/images/image5.jpg", ":/images/image6.jpg", ":/images/image7.jpg"};

    QStringList names = {"Nhân viên A", "Nhân viên B", "Nhân viên C", "Nhân viên D", "Nhân viên E", "Quản lý 1", "Quản lý 2"};
    QStringList roles = {"Nhân viên", "Nhân viên", "Nhân viên", "Nhân viên", "Nhân viên", "Quản lý", "Quản lý"};
    QStringList emails = {"a@congty.com", "b@congty.com", "c@congty.com", "d@congty.com", "e@congty.com", "ql1@congty.com", "ql2@congty.com"};
    QStringList phones = {"0901 111 111", "0902 222 222", "0903 333 333", "0904 444 444", "0905 555 555", "0906 666 666", "0907 777 777"};

    int row = 0;
    int col = 0;
    const int maxColumns = 4;

    for (int i = 0; i < avatars.size(); ++i)
    {
        EmployeeCard *card = new EmployeeCard(this);
        card->setData(avatars.at(i), names.at(i), roles.at(i), "✉️ " + emails.at(i), "📞 " + phones.at(i));
        gridLayout->addWidget(card, row, col);
        ++col;
        if (col >= maxColumns)
        {
            col = 0;
            ++row;
        }
    }

    ui->lblAvatar->setCursor(Qt::PointingHandCursor);
    ui->lblUserName->setCursor(Qt::PointingHandCursor);
    ui->lblUserRole->setCursor(Qt::PointingHandCursor);
    ui->lblDropdown->setCursor(Qt::PointingHandCursor);

    ui->lblAvatar->installEventFilter(this);
    ui->lblUserName->installEventFilter(this);
    ui->lblUserRole->installEventFilter(this);
    ui->lblDropdown->installEventFilter(this);

    // profile avatar dropbox
    QWidget *avatarBox = new QWidget(this);
    avatarBox->setLayout(ui->horizontalLayout_UserInfo);

    // Embed EmployeesWidget into the HR page layout
    QLayout *hrLayout = ui->pageHR->layout();
    if (hrLayout)
    {
        QLayoutItem *item;
        while ((item = hrLayout->takeAt(0)) != nullptr)
        {
            if (item->widget())
                delete item->widget();
            delete item;
        }
    }
    else
    {
        hrLayout = new QVBoxLayout(ui->pageHR);
    }
    EmployeesWidget *employeesWidget = new EmployeesWidget(this);
    hrLayout->addWidget(employeesWidget);
}

Dashboard_View::~Dashboard_View()
{
    delete ui;
}

Dashboard_Control *Dashboard_View::getController() const
{
    return controller;
}

bool Dashboard_View::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        if (watched == ui->lblAvatar || watched == ui->lblUserName ||
            watched == ui->lblUserRole || watched == ui->lblDropdown)
        {
            if (ui && ui->stackedWidget)
                ui->stackedWidget->setCurrentIndex(0);
            if (controller)
                emit controller->profilePageClicked();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void Dashboard_View::on_btnMenu_Overview_clicked() {
    if (ui && ui->stackedWidget) ui->stackedWidget->setCurrentWidget(ui->pageOverview);
}

void Dashboard_View::on_btnMenu_HR_clicked() {
    if (ui && ui->stackedWidget) ui->stackedWidget->setCurrentWidget(ui->pageHR);
}

void Dashboard_View::on_btnMenu_Timekeep_clicked() {
    if (ui && ui->stackedWidget) ui->stackedWidget->setCurrentWidget(ui->pageTimekeep);
}

void Dashboard_View::on_btnMenu_Salary_clicked() {
    if (ui && ui->stackedWidget) ui->stackedWidget->setCurrentWidget(ui->pageSalary);
}

void Dashboard_View::on_btnMenu_Report_clicked() {
    if (ui && ui->stackedWidget) ui->stackedWidget->setCurrentWidget(ui->pageReport);
}

void Dashboard_View::on_btnMenu_Settings_clicked() {
    if (ui && ui->stackedWidget) ui->stackedWidget->setCurrentWidget(ui->pageSettings);
}

void Dashboard_View::on_btnLogout_clicked()
{
    if (controller)
        emit controller->logoutSubmitted();
}
