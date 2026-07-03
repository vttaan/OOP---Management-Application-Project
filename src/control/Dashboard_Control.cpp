#include "Dashboard_Control.h"

Dashboard_Control::Dashboard_Control(QObject *parent)
    :QObject(parent), view(nullptr), currentUser(nullptr){

}
Dashboard_Control::~Dashboard_Control() {
    if (view) delete view;
    if (currentUser) delete currentUser;
}
void Dashboard_Control::init(){
    view->show();
}

Dashboard_View* Dashboard_Control::getView()  {
    return this->view;
}

void Dashboard_Control::setView(Dashboard_View* view) {
    this->view = view;
}