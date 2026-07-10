#include "global.h"
#ifndef VIEW_NAVIGATOR_H
#define VIEW_NAVIGATOR_H
// #include "Login_View.h"
// #include "Profile_View.h"
// #include "Dashboard_View.h"
#include "control/Control_Navigator.h"

namespace Ui {
class View_Navigator;
}

class View_Navigator : public QMainWindow
{
    Q_OBJECT
private:

    QWidget* currentWindow;

    Control_Navigator* controller;
public:
    View_Navigator(Control_Navigator* controller, QWidget *parent = nullptr);

    Login_View* loginPage = nullptr;
    Dashboard_View* dashboardPage = nullptr;
    Profile_View* profilePage = nullptr;
    EmployeesWidget* employeePage = nullptr;
    Control_Navigator* getController();
    QWidget* getWindow();
    Ui::View_Navigator* getUI();
    void setPageIndex(int index);
    Ui::View_Navigator* ui;
    ~View_Navigator();

signals:
    void logoutSubmitted();
};

#endif // VIEW_NAVIGATOR_H
