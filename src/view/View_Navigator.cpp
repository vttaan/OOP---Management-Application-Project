#include "View_Navigator.h"
#include "ui_View_Navigator.h"
#include "Login_View.h"
#include "Profile_View.h"
#include "main_view.h"
#include "control/Login_Control.h"
#include <QStackedWidget>

View_Navigator::View_Navigator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::View_Navigator)
{
    ui->setupUi(this);

    // Remove default placeholder pages created by Qt Designer
    while (ui->stackedWidget->count() > 0) {
        QWidget* widget = ui->stackedWidget->widget(0);
        ui->stackedWidget->removeWidget(widget);
        delete widget;
    }

    // create pages
    Main_View* dashboard_page = new Main_View(this);
    Profile_View* profile_page = new Profile_View(this);
    Login_Control* login_control = new Login_Control(this);
    Login_View* login_page = login_control->getView();

    // add pages
    // index note for each page
    ui->stackedWidget->addWidget(login_page); // index 0
    ui->stackedWidget->addWidget(dashboard_page); // index 1
    ui->stackedWidget->addWidget(profile_page); // index 2
    // default : login page
    ui->stackedWidget->setCurrentIndex(0);

    // connect signals
    connect(login_control, &Login_Control::loginSuccessful, this, [this]() {
        qDebug() << "Dashboard\n";
        ui->stackedWidget->setCurrentIndex(1);
    });

    connect(dashboard_page, &Main_View::logoutSubmitted, this, [this, login_page]() {
        qDebug() << "Log out\n";
        ui->stackedWidget->setCurrentIndex(0);
        login_page->clearInputs();
        emit logoutSubmitted();
    });

    connect(dashboard_page, &Main_View::profilePageClicked, this, [this]() {
        qDebug() << "Profile page\n";
        ui->stackedWidget->setCurrentIndex(2);
    } );

    connect(profile_page, &Profile_View::backToPrevious, this, [this]() {
        qDebug() << "Dashboard\n";
        ui->stackedWidget->setCurrentIndex(1);
    });



}

View_Navigator::~View_Navigator()
{
    delete ui;
}

