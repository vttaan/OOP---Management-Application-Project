#include "Profile_View.h"
#include "ui_Profile_View.h"

Profile_View::Profile_View(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Profile_View)
{
    ui->setupUi(this);
}

Profile_View::~Profile_View()
{
    delete ui;
}
