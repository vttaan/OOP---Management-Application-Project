#include "View_Navigator.h"
#include "ui_View_Navigator.h"
#include "control/Control_Navigator.h"
#include "Login_View.h"
#include "Dashboard_View.h"
#include "Profile_View.h"
#include <QStackedWidget>
#include "view/Profile_View.h"

View_Navigator::View_Navigator(Control_Navigator* controller, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::View_Navigator)
    , controller(controller)
    , loginPage(new Login_View(controller->loginController))
    , dashboardPage(new Dashboard_View(controller->dashboardController))
    , profilePage(new Profile_View(controller->profileController))
{
    ui->setupUi(this);

    // Remove default placeholder pages created by Qt Designer
    while (ui->stackedWidget->count() > 0) {
        QWidget* widget = ui->stackedWidget->widget(0);
        ui->stackedWidget->removeWidget(widget);
        delete widget;
    }

    // Set the views on the controllers
    // controller->loginController->setView(this->loginPage);
    // controller->profileController->setView(this->profilePage);
    // controller->dashboardController->setView(this->dashboardPage);

    // add pages
    // index note for each page
    ui->stackedWidget->addWidget(loginPage); // index 0
    ui->stackedWidget->addWidget(dashboardPage); // index 1
    ui->stackedWidget->addWidget(profilePage); // index 2
    // default : login page
    ui->stackedWidget->setCurrentIndex(0);

    // Navigation is managed directly by Control_Navigator now.



}

QWidget* View_Navigator::getWindow() { return currentWindow; }

Control_Navigator* View_Navigator::getController() { return controller; }
Ui::View_Navigator* View_Navigator::getUI() { return ui; }

void View_Navigator::setPageIndex(int index) {
    if (ui && ui->stackedWidget) {
        ui->stackedWidget->setCurrentIndex(index);
    }
}

View_Navigator::~View_Navigator()
{
    delete ui;
}

