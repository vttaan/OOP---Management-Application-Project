#include "Dashboard_View.h"
#include "ui_Dashboard_View.h"
#include "control/Dashboard_Control.h"

Dashboard_View::Dashboard_View(Dashboard_Control *controller, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Dashboard_View),
    controller(controller)
{
    ui->setupUi(this);
}

Dashboard_View::~Dashboard_View()
{
    delete ui;
}

Dashboard_Control *Dashboard_View::getController() const {
    return controller;
}

void Dashboard_View::setController(Dashboard_Control *ctrl) {
    controller = ctrl;
}

void Dashboard_View::embedHRPage(QWidget *hrWidget) {
}

void Dashboard_View::showHRPage() {
}

void Dashboard_View::embedWidgetIntoPage(int index, QWidget* widget) {
    if (!widget) return;

    QWidget* targetPage = ui->stackedWidget->widget(index);
    if (targetPage) {
        targetPage->layout()->addWidget(widget);
    }
}

bool Dashboard_View::eventFilter(QObject *watched, QEvent *event) {
    return QWidget::eventFilter(watched, event);
}

void Dashboard_View::switchPage(int index) {
    ui->stackedWidget->setCurrentIndex(index);

    QString defaultStyle = "QPushButton { text-align: left; padding-left: 20px; font-size: 15px; font-weight: 500; color: #637381; border: none; border-radius: 8px; height: 45px; margin: 5px 15px; } "
                           "QPushButton:hover { background-color: #f4f6f8; color: #212b36; }";

    QString activeStyle = "QPushButton { text-align: left; padding-left: 20px; font-size: 15px; font-weight: bold; color: #1a73e8; background-color: #e8f0fe; border: none; border-radius: 8px; height: 45px; margin: 5px 15px; }";

    ui->btnMenu_Overview->setStyleSheet(defaultStyle);
    ui->btnMenu_HR->setStyleSheet(defaultStyle);
    ui->btnMenu_Timekeep->setStyleSheet(defaultStyle);
    ui->btnMenu_Salary->setStyleSheet(defaultStyle);
    ui->btnMenu_Report->setStyleSheet(defaultStyle);
    ui->btnMenu_Settings->setStyleSheet(defaultStyle);

    switch(index) {
    case 0: ui->btnMenu_Overview->setStyleSheet(activeStyle); break;
    case 1: ui->btnMenu_HR->setStyleSheet(activeStyle); break;
    case 2: ui->btnMenu_Timekeep->setStyleSheet(activeStyle); break;
    case 3: ui->btnMenu_Salary->setStyleSheet(activeStyle); break;
    case 4: ui->btnMenu_Report->setStyleSheet(activeStyle); break;
    case 5: ui->btnMenu_Settings->setStyleSheet(activeStyle); break;
    }
}

void Dashboard_View::toggleSidebar() {
    if (ui->frmSidebar->isVisible()) {
        ui->frmSidebar->hide();
    } else {
        ui->frmSidebar->show();
    }
}

void Dashboard_View::on_btnMenu_Overview_clicked() {
    switchPage(0);
}
void Dashboard_View::on_btnMenu_HR_clicked() {
    switchPage(1);
    if (controller) {
        emit controller->employeeClicked();
    }
}
void Dashboard_View::on_btnMenu_Timekeep_clicked() {
    switchPage(2);
}
void Dashboard_View::on_btnMenu_Salary_clicked() {
    switchPage(3);
}
void Dashboard_View::on_btnMenu_Report_clicked() {
    switchPage(4);
}
void Dashboard_View::on_btnMenu_Settings_clicked() {
    switchPage(5);
}
void Dashboard_View::on_btnLogout_clicked() {
    if (controller) {
        emit controller->logoutSubmitted();
    }
}