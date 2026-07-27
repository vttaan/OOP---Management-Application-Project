#include "global.h"

#include "control/Control_Navigator.h"
#include "view/View_Navigator.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFile styleFile(":/styles/styles.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream stream(&styleFile);
        app.setStyleSheet(stream.readAll());
        styleFile.close();
        qDebug() << "Load QSS File success!";
    }
    else qDebug() << "Can not load QSS File!";

    if (QFontDatabase::addApplicationFont(":/fonts/inter.ttf") != -1) {
        qDebug() << "Load Inter font success!";
    } else {
        qDebug() << "Failed to load Inter font!";
    }

    Control_Navigator *appWindow = new Control_Navigator();
    if (appWindow && appWindow->viewWindow) {

        appWindow->viewWindow->setWindowTitle("Optimus - Phần mềm Quản Lý Nhân Sự");

        appWindow->viewWindow->setWindowIcon(QIcon(":/images/logo.png"));

        appWindow->viewWindow->showMaximized();
    }

    int res = app.exec();

    Database::getInstance()->closeConnect();
    return res;
}