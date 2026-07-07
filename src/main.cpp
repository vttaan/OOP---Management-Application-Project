#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
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
  else
  {
    qDebug() << "Can not load QSS File!";
  }

  Control_Navigator *appWindow = new Control_Navigator();
  if (appWindow && appWindow->viewWindow)
    appWindow->viewWindow->show();

    int res = app.exec();

    Database::getInstance()->closeConnect();
    return res;
}
