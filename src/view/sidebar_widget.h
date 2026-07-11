#ifndef SIDEBAR_WIDGET_H
#define SIDEBAR_WIDGET_H

#include <QWidget>

namespace Ui {
class Sidebar_Widget;
}

class Sidebar_Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Sidebar_Widget(QWidget *parent = nullptr);
    ~Sidebar_Widget();

signals:
    void menuClicked(int pageIndex);
    void logoutClicked();

private:
    Ui::Sidebar_Widget *ui;
    void updateButtonStyles(int activeIndex); // Hàm tô màu xanh cho nút
};

#endif // SIDEBAR_WIDGET_H