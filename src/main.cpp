#include <QApplication>
#include <QWidget>
#include <QFile>
#include <QTextStream>

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

    View_Navigator appWindow;
    appWindow.show();

    return a.exec();
}