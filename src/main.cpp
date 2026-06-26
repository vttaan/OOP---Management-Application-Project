#include <QApplication>
#include <QWidget>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    QWidget window;
    window.setWindowTitle("Management-App");
    window.resize(800, 600);
    window.show();
    return a.exec();
}