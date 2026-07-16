#include "global.h"
#include "main_view.h"
#include "ui_main_view.h"
#include "employeecard.h"

Main_View::Main_View(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Overview_view)
{
    ui->setupUi(this);

    gridLayoutEmployees = new QGridLayout(ui->scrollAreaWidgetContents);
    gridLayoutEmployees->setSpacing(20);
    currentRow = 0;
    currentCol = 0;

    connect(ui->btnMenu_Overview, &QPushButton::clicked, this, &Main_View::menuOverviewClicked);
    connect(ui->btnMenu_HR, &QPushButton::clicked, this, &Main_View::menuHRClicked);
    connect(ui->btnMenu_Timekeep, &QPushButton::clicked, this, &Main_View::menuTimekeepClicked);
    connect(ui->btnMenu_Salary, &QPushButton::clicked, this, &Main_View::menuSalaryClicked);
    connect(ui->btnMenu_Report, &QPushButton::clicked, this, &Main_View::menuReportClicked);
    connect(ui->btnMenu_Settings, &QPushButton::clicked, this, &Main_View::menuSettingsClicked);
    connect(ui->btnToggleSidebar, &QPushButton::clicked, this, &Main_View::toggleSidebarClicked);
    ui->lblAvatar->setCursor(Qt::PointingHandCursor);
    ui->lblUserName->setCursor(Qt::PointingHandCursor);
    ui->lblUserRole->setCursor(Qt::PointingHandCursor);
    ui->lblDropdown->setCursor(Qt::PointingHandCursor);

    ui->lblAvatar->installEventFilter(this);
    ui->lblUserName->installEventFilter(this);
    ui->lblUserRole->installEventFilter(this);
    ui->lblDropdown->installEventFilter(this);
}

Main_View::~Main_View()
{
    delete ui;
}

void Main_View::switchPage(int pageIndex)
{
    ui->stackedWidget->setCurrentIndex(pageIndex);

    QString defaultStyle = "QPushButton { text-align: left; padding-left: 20px; font-size: 15px; font-weight: 500; color: #637381; border: none; border-radius: 8px; height: 45px; margin: 5px 15px; } "
                           "QPushButton:hover { background-color: #f4f6f8; color: #212b36; }";

    QString activeStyle = "QPushButton { text-align: left; padding-left: 20px; font-size: 15px; font-weight: bold; color: #1a73e8; background-color: #e8f0fe; border: none; border-radius: 8px; height: 45px; margin: 5px 15px; }";

    ui->btnMenu_Overview->setStyleSheet(defaultStyle);
    ui->btnMenu_HR->setStyleSheet(defaultStyle);
    ui->btnMenu_Timekeep->setStyleSheet(defaultStyle);
    ui->btnMenu_Salary->setStyleSheet(defaultStyle);
    ui->btnMenu_Report->setStyleSheet(defaultStyle);
    ui->btnMenu_Settings->setStyleSheet(defaultStyle);
    switch(pageIndex) {
    case 0: ui->btnMenu_Overview->setStyleSheet(activeStyle); break;
    case 1: ui->btnMenu_HR->setStyleSheet(activeStyle); break;
    case 2: ui->btnMenu_Timekeep->setStyleSheet(activeStyle); break;
    case 3: ui->btnMenu_Salary->setStyleSheet(activeStyle); break;
    case 4: ui->btnMenu_Report->setStyleSheet(activeStyle); break;
    case 5: ui->btnMenu_Settings->setStyleSheet(activeStyle); break;
    }
}

bool Main_View::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (watched == ui->lblAvatar || watched == ui->lblUserName ||
            watched == ui->lblUserRole || watched == ui->lblDropdown) {

            emit profileClicked();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

