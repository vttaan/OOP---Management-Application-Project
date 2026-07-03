#ifndef VIEW_NAVIGATOR_H
#define VIEW_NAVIGATOR_H

#include <QMainWindow>

namespace Ui {
class View_Navigator;
}

class View_Navigator : public QMainWindow
{
    Q_OBJECT

public:
    explicit View_Navigator(QWidget *parent = nullptr);
    ~View_Navigator();

private:
    Ui::View_Navigator *ui;
signals:
    void logoutSubmitted();
};

#endif // VIEW_NAVIGATOR_H
