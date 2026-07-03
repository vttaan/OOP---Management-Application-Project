#include <QApplication>
#include <QWidget>
#include <QFile>
#include <QTextStream>

#include "control/Control_Navigator.h"
#include "view/View_Navigator.h"

// #include "control/Login_Control.h"
// int main(int argc, char *argv[])
// {
//     QApplication app(argc, argv);

//     // set up qss file
//     QFile styleFile(":styles/styles.qss");
//     if (styleFile.open(QFile::ReadOnly | QFile::Text))
//     {
//         QTextStream stream(&styleFile);
//         app.setStyleSheet(stream.readAll());
//         styleFile.close();
//         qDebug() << "Load QSS File success";
//     }
//     else
//         qDebug() << "Can not load QSS File";
//     //

//     Login_Control loginController;
//     loginController.init();
//     return app.exec();
// }

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Load and apply stylesheet globally
    QFile styleFile(":/styles/styles.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream stream(&styleFile);
        a.setStyleSheet(stream.readAll());
        styleFile.close();
        qDebug() << "Load QSS File success";
    }
    else
    {
        qDebug() << "Cannot load QSS File from path";
    }

    Control_Navigator* appWindow = new Control_Navigator;
    appWindow->viewWindow->show();

    return a.exec();
}