#ifndef SIDEBAR_WIDGET_H
#define SIDEBAR_WIDGET_H

#include <QWidget>

namespace Ui { class Sidebar_Widget; }

class Sidebar_Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Sidebar_Widget(QWidget *parent = nullptr);
    ~Sidebar_Widget();
    QString getNormalStyle() {return this->normalStyle;}
    QString getActiveStyle() {return this->activeStyle;}
    void setPermission(const bool& permitted);
    void updateButtonStyles(int mainIndex);

signals:
    void menuClicked(int pageIndex);
    void logoutClicked();

private:
    Ui::Sidebar_Widget *ui;
    // mainIndex is tab in sidebar, subIndex is tab in Schedule
    void initUI();
    // CSS for normal button
    const QString normalStyle = "QPushButton { "
                                "   text-align: left; "
                                "   padding-left: 35px; "
                                "   font-size: 14px; "
                                "   font-weight: 600; "
                                "   color: #475467; "
                                "   background-color: transparent; "
                                "   border: none; "
                                "   border-radius: 8px; "
                                "   height: 45px; "
                                "   margin: 4px 16px; "
                                "} "
                                "QPushButton:hover { "
                                "   background-color: #F1F5F9; "
                                "}";
    // CSS for clicked button
    const QString activeStyle =
        "QPushButton { "
        "   text-align: left; "
        "   padding-left: 35px; "
        "   font-size: 14px; "
        "   font-weight: bold; "
        "   color: #1A73E8; "
        "   background-color: #E8F0FE; "
        "   border: none; "
        "   border-radius: 8px; "
        "   height: 45px; "
        "   margin: 4px 16px; "
        "}";
};

#endif