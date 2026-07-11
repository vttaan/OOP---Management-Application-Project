#include "global.h"
#include "Control_Navigator.h"
#include "Login_Control.h"
#include "Dashboard_Control.h"
#include "Profile_Control.h"
#include "view/View_Navigator.h"
#include "view/Dashboard_View.h"
#include "utils/SessionManage.h"
#include "overview_control.h"
#include "view/overview_view.h"
#include "view/employeeswidget.h"
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
    this->viewWindow = new View_Navigator(this); // Initialize viewWindow AFTER controllers
    this->overviewController = new Overview_Control(this);
    this->dashboardController->init();
    this->overviewController->init();
    this->dashboardController->getView()->embedWidgetIntoPage(0, this->overviewController->getView());
    this->dashboardController->getView()->embedWidgetIntoPage(1, this->employeeController->getView());
    QObject::connect(this->overviewController->getView(), SIGNAL(toggleSidebarRequested()),
                     this->dashboardController->getView(), SLOT(toggleSidebar()));
    QObject::connect(this->loginController, &Login_Control::loginSuccessful,
                     this->viewWindow, [this]() {
        this->switchTab(1); // Switch to Dashboard (index 1)
        this->profileController->currentSession = this->currentSession;
        this->profileController->loadUserData();
        //qDebug() << "current user: " << this->currentSession->getCurrentUser()->getName();
        // the whole app's session is updated
    });

    QObject::connect(this->dashboardController, &Dashboard_Control::logoutSubmitted,
                     this->viewWindow, [this]() {
        this->switchTab(0); // Switch to Login (index 0)
        qDebug() << "logout";
        this->loginController->init();
    });

    QObject::connect(this->dashboardController, &Dashboard_Control::profilePageClicked,
                     this->viewWindow, [this]() {
        this->switchTab(2); // Switch to Profile (index 2)
        //this->profileController->hand
        //qDebug() << this->profileController->currentSession->getCurrentUser()->getName();
        // no need to load user data for profile since its session already pointed to the whole app's session
    });

    QObject::connect(this->profileController, &Profile_Control::backToPrevious,
                     this->viewWindow, [this]() {
        this->switchTab(1); // Switch to Dashboard (index 1)
    });

    QObject::connect(this->employeeController, &Employee_Control::profilePageClicked,
                     this->viewWindow, [this]() {
                         this->switchTab(2); // from Employee  switch to Profile (index 2)
                        //qDebug() << this->profileController->currentSession->getCurrentUser()->getName();
                     });
    QObject::connect(this->employeeController, &Employee_Control::backToDashBoard,
                     this->viewWindow, [this]() {
                         this->switchTab(1); // from Employee switch to Dashboard (index 1)
                     });
    QObject::connect(this->dashboardController, &Dashboard_Control::employeeClicked,
                     this->viewWindow, [this]() {
                        this->switchTab(1); // from Dashboard (index 1) switch to Employee
                        this->viewWindow->dashboardPage->showHRPage();
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
