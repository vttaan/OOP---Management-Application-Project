#include "global.h"
#include "View_Navigator.h"
#include "ui_View_Navigator.h"
#include "control/Control_Navigator.h"
#include "Login_View.h"
#include "Dashboard_View.h"
#include "Profile_View.h"
#include "view/employeeswidget.h"
#include "view/Profile_View.h"

View_Navigator::View_Navigator(Control_Navigator* controller, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::View_Navigator)
    , controller(controller)
    , loginPage(new Login_View(controller->loginController))
    , dashboardPage(new Dashboard_View())
    , profilePage(new Profile_View(controller->profileController))
    , employeePage(new EmployeesWidget())
    , schedulePage(new Schedule_View())
    , salaryPage(new Salary_View())
{
    ui->setupUi(this);

    // Remove default placeholder pages created by Qt Designer
    while (ui->stackedWidget->count() > 0) {
        QWidget* widget = ui->stackedWidget->widget(0);
        ui->stackedWidget->removeWidget(widget);
        delete widget;
    }


    // Set the views on the controllers
    controller->loginController->setView(loginPage);
    controller->profileController->setView(profilePage);
    controller->dashboardController->setView(dashboardPage);
    controller->employeeController->setView(employeePage);
    controller->scheduleController->setView(schedulePage);
    controller->salaryController->setView(salaryPage);
    // add pages
    // index note for each page
    ui->stackedWidget->addWidget(loginPage); // index 0
    ui->stackedWidget->addWidget(dashboardPage); // index 1
    ui->stackedWidget->addWidget(profilePage); // index 2
    ui->stackedWidget->addWidget(employeePage); // index 3
    ui->stackedWidget->addWidget(schedulePage); // index 4
    ui->stackedWidget->addWidget(salaryPage); //index 5
    //ui->stackedWidget->addWidget(schedulePage);
    // default : login page
    ui->stackedWidget->setCurrentIndex(0);

    // Navigation is managed directly by Control_Navigator now.



    // default
    setPageIndex(0);

}

Sidebar_Widget* View_Navigator::getSideBar() {
    return ui->widget;
}

QWidget* View_Navigator::getWindow() { return currentWindow; }

Control_Navigator* View_Navigator::getController() { return controller; }
Ui::View_Navigator* View_Navigator::getUI() { return ui; }

void View_Navigator::setPageIndex(int index) {
    if(ui && ui->stackedWidget) {
        int stackedIndex = index;
        if (index >= 4 && index <= 6) {
            stackedIndex = 4; // Schedule page
        } else if (index == 7) {
            stackedIndex = 5; // Salary page
        }
        ui->stackedWidget->setCurrentIndex(stackedIndex);
        // login and profile can not show side bar
        if(index == 0 || index == 2) ui->widget->hide();
        else ui->widget->show();
    }
}

View_Navigator::~View_Navigator()
{
    delete ui;
}

