#include "Dashboard_Control.h"
#include "view/Dashboard_View.h"

Dashboard_Control::Dashboard_Control(QObject *parent)
    :QObject(parent), view(nullptr), currentSession(nullptr){

}
Dashboard_Control::~Dashboard_Control() {
    if (view) delete view;
    if (currentSession) delete currentSession;
}
void Dashboard_Control::init(){
    this->getView()->show();
}

Dashboard_View* Dashboard_Control::getView()  {
    return this->view;
}

void Dashboard_Control::setView(Dashboard_View* view) {
    this->view = view;
    if (this->view) {
        this->getView()->setController(this);
    }
}