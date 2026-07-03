#ifndef CONTROL_NAVIGATOR_H
#define CONTROL_NAVIGATOR_H
#include "Login_Control.h"
#include "Profile_Control.h"
#include "Dashboard_Control.h"
//#include "view/View_Navigator.h"

class View_Navigator;

class Control_Navigator
{
public:
    Login_Control* loginController = nullptr;
    Profile_Control* profileController = nullptr;
    Dashboard_Control* dashboardController = nullptr;

    View_Navigator* viewWindow = nullptr;

    Control_Navigator();
    void initUIByRole();
    void switchTab(int index);
    void handleLogOut();

    ~Control_Navigator();
};

#endif // CONTROL_NAVIGATOR_H
