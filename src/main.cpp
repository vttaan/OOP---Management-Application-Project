#include <QApplication>
#include <QWidget>
#include "model/Login_Model.h"
#include "utils/Security.h"
#include <QDebug>
int main(int argc, char* argv[]) {
   QApplication a(argc, argv);

  /*  QWidget window;
    window.setWindowTitle("Management-App");
    window.resize(800, 600);
    window.show();*/

    QString q = Security::hashPassword("06022003");
    qDebug() << q;

    //return a.exec();


    return 0;
}