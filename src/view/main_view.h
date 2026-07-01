#ifndef MAIN_VIEW_H
#define MAIN_VIEW_H
#include<QEvent>
#include <QWidget>

namespace Ui {
class Main_View;
}

class Main_View : public QWidget
{
    Q_OBJECT

public:
    explicit Main_View(QWidget *parent = nullptr);
    ~Main_View();
protected:
    bool eventFilter(QObject*watched,QEvent*event)override;
private:
    Ui::Main_View *ui;
};

#endif
