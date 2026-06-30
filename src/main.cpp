#include <QApplication>
#include <QWidget>
#include <QFile>
#include <QTextStream>

#include "control/Login_Control.h"
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // set up qss file
    QFile styleFile(":styles/styles.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream stream(&styleFile);
        app.setStyleSheet(stream.readAll());
        styleFile.close();
        qDebug() << "Load QSS File success";
    }
    else
        qDebug() << "Can not load QSS File";
    //

    Login_Control loginController;
    loginController.init();
    return app.exec();
}