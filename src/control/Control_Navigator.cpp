#include "global.h"
#include "Control_Navigator.h"
#include "Login_Control.h"
#include "Dashboard_Control.h"
#include "Profile_Control.h"
#include "Employee_Control.h"
#include "overview_control.h"
#include "view/View_Navigator.h"
#include "view/Dashboard_View.h"
#include "utils/SessionManage.h"
#include "view/employeeswidget.h"
#include "view/sidebar_widget.h"
#include "view/overview_view.h"

Control_Navigator::Control_Navigator()
{
    this->currentSession = new SessionManager();

    this->dashboardController = new Dashboard_Control(this);
    this->dashboardController->currentSession = this->currentSession;

    this->loginController = new Login_Control(this);
    this->loginController->currentSession = this->currentSession;

    this->profileController = new Profile_Control(this);
    this->profileController->currentSession = this->currentSession;

    this->employeeController = new Employee_Control(this);
    this->overviewController = new Overview_Control(this);

    this->viewWindow = new View_Navigator(this); // Initialize viewWindow AFTER controllers

    this->dashboardController->init();
    this->overviewController->init();

    Sidebar_Widget* sidebar = new Sidebar_Widget();
    this->dashboardController->getView()->addSidebar(sidebar);

    QObject::connect(sidebar, &Sidebar_Widget::menuClicked,
                     this->dashboardController->getView(), &Dashboard_View::switchPage);


    this->dashboardController->getView()->embedWidgetIntoPage(0, this->overviewController->getView());
    this->dashboardController->getView()->embedWidgetIntoPage(1, this->employeeController->getView());

    QObject::connect(this->overviewController->getView(), SIGNAL(toggleSidebarRequested()),
                     this->dashboardController->getView(), SLOT(toggleSidebar()));


    QObject::connect(this->loginController, &Login_Control::loginSuccessful,
                     this->viewWindow, [this]() {
                         this->switchTab(1); // Switch to Dashboard (index 1)
                         this->profileController->currentSession = this->currentSession;
                         this->profileController->loadUserData();
                     });


    auto logoutHandler = [this]() {
        this->switchTab(0); // Switch to Login (index 0)
        qDebug() << "logout";
        this->loginController->init();
    };

    QObject::connect(sidebar, &Sidebar_Widget::logoutClicked, this->viewWindow, logoutHandler);

    QObject::connect(this->dashboardController, &Dashboard_Control::logoutSubmitted, this->viewWindow, logoutHandler);


    QObject::connect(this->dashboardController, &Dashboard_Control::employeeClicked,
                     this->viewWindow, [this]() {
                         this->switchTab(1); // from Dashboard (index 1) switch to Employee
                         this->employeeController->init();
                     });
}

void Control_Navigator::switchTab(int index) {
    if (this->viewWindow) {
        this->viewWindow->setPageIndex(index);
    }
}

void Control_Navigator::initUIByRole() {
    // Implement UI initialization by role here
}

void Control_Navigator::handleLogOut() {
    // Implement logout logic here
}

Control_Navigator::~Control_Navigator() {
    delete currentSession;
    delete viewWindow;
    delete loginController;
    delete profileController;
    delete dashboardController;
    delete employeeController;
    delete overviewController;
    currentSession = nullptr;
    viewWindow = nullptr;
    loginController = nullptr;
    profileController = nullptr;
    dashboardController = nullptr;
    employeeController = nullptr;
    overviewController = nullptr;
}