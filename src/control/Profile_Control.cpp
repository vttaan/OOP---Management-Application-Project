#include "Profile_Control.h"
#include "../view/Profile_View.h"

Profile_Control::Profile_Control(QObject *parent)
    :QObject(parent), view(nullptr), currentUser(nullptr){

}
Profile_Control::~Profile_Control() {
    if (view) delete view;
    if (currentUser) delete currentUser;
}
void Profile_Control::init(){
    view->show();
}

Profile_View* Profile_Control::getView()  {
    return this->view;
}

void Profile_Control::setView(Profile_View* view) {
    this->view = view;
}