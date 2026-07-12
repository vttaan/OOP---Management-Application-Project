#include "global.h"
#include "Control_Navigator.h"
#include "Login_Control.h"
#include "Dashboard_Control.h"
#include "Profile_Control.h"
#include "view/View_Navigator.h"
#include "view/Dashboard_View.h"
#include "utils/SessionManage.h"
#include "view/employeeswidget.h"

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

    this->scheduleController = new Schedule_Control(this);

    this->viewWindow = new View_Navigator(this); // Initialize viewWindow AFTER controllers

    // switch tab side bar do all
    if(this->viewWindow->getSideBar()) {
        // switch tab
        QObject::connect(this->viewWindow->getSideBar(), &Sidebar_Widget::menuClicked,
                         this, &Control_Navigator::switchTab);
        // logout
        QObject::connect(this->viewWindow->getSideBar(), &Sidebar_Widget::logoutClicked,
                         this, [this](){
            this->switchTab(0);
            this->loginController->init();
        });
    }

    QObject::connect(this->loginController, &Login_Control::loginSuccessful,
                     this->viewWindow, [this]() {
        this->switchTab(1); // Switch to Dashboard (index 1)
        this->profileController->currentSession = this->currentSession;
        this->profileController->loadUserData();
        //qDebug() << "current user: " << this->currentSession->getCurrentUser()->getName();
        // the whole app's session is updated
    });

    QObject::connect(this->dashboardController, &Dashboard_Control::profilePageClicked,
                     this->viewWindow, [this]() {
        this->switchTab(2); // Switch to Profile (index 2)
        //this->profileController->hand
        //qDebug() << this->profileController->currentSession->getCurrentUser()->getName();
        // no need to load user data for profile since its session already pointed to the whole app's session
    });

    // switch from profile back previous
    QObject::connect(this->profileController, &Profile_Control::backToPrevious,
                     this->viewWindow, [this]() {
        this->switchTab(1); // Switch to Dashboard (index 1)
    });

    QObject::connect(this->employeeController, &Employee_Control::profilePageClicked,
                     this->viewWindow, [this]() {
                         this->switchTab(2); // from Employee  switch to Profile (index 2)
                        //qDebug() << this->profileController->currentSession->getCurrentUser()->getName();
                     });
}

void Control_Navigator::switchTab(int index) {
    // load data before switch tab
    switch(index) {
    case 1:
        this->dashboardController->init();
        break;
    case 2:
        this->profileController->init();
        break;
    case 3:
        this->employeeController->init();
        break;
    case 4:
        this->scheduleController->load();
        break;
    }

    // show view tab
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
    delete scheduleController;
    currentSession = nullptr;
    viewWindow = nullptr;
    loginController = nullptr;
    profileController = nullptr;
    dashboardController = nullptr;
    employeeController = nullptr;
    scheduleController = nullptr;
}

