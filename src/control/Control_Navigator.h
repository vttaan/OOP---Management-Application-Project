#include "global.h"
#ifndef CONTROL_NAVIGATOR_H
#define CONTROL_NAVIGATOR_H
#include "utils/SessionManage.h"
#include "Login_Control.h"
#include "Dashboard_Control.h"
#include "Profile_Control.h"
#include "Employee_Control.h"
//#include "view/View_Navigator.h"
class Overview_Control;
class View_Navigator;
class Control_Navigator : public QObject
{
    Q_OBJECT
public:
    Login_Control* loginController = nullptr;
    Profile_Control* profileController = nullptr;
    Dashboard_Control* dashboardController = nullptr;
    Employee_Control * employeeController = nullptr;
    View_Navigator* viewWindow = nullptr;
    SessionManager* currentSession = nullptr;
    Overview_Control* overviewController = nullptr;
    Control_Navigator();
    void initUIByRole();
    void switchTab(int index);
    void handleLogOut();

    ~Control_Navigator();
};

#endif // CONTROL_NAVIGATOR_H
