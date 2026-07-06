#include "Control_Navigator.h"
#include "view/View_Navigator.h"


Control_Navigator::Control_Navigator()
{
    this->dashboardController = new Dashboard_Control();
    this->loginController = new Login_Control();
    this->profileController = new Profile_Control();

    this->viewWindow = new View_Navigator(this);

    this->loginController->setParent(this->viewWindow);
    this->profileController->setParent(this->viewWindow);
    this->dashboardController->setParent(this->viewWindow);

    QObject::connect(this->loginController, &Login_Control::loginSuccessful,
                     this->viewWindow, [this]() {
        this->switchTab(1); // Switch to Dashboard (index 1)
    });

    QObject::connect(this->dashboardController, &Dashboard_Control::logoutSubmitted,
                     this->viewWindow, [this]() {
        this->switchTab(0); // Switch to Login (index 0)
        this->loginController->getView()->clearInputs();
    });

    QObject::connect(this->dashboardController, &Dashboard_Control::profilePageClicked,
                     this->viewWindow, [this]() {
        this->switchTab(2); // Switch to Profile (index 2)
    });

    QObject::connect(this->profileController, &Profile_Control::backToPrevious,
                     this->viewWindow, [this]() {
        this->switchTab(1); // Switch to Dashboard (index 1)
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
    delete viewWindow;
    delete loginController;
    delete profileController;
    delete dashboardController;
    delete employeeController;
    viewWindow = nullptr;
    loginController = nullptr;
    profileController = nullptr;
    dashboardController = nullptr;
    employeeController = nullptr;
}
