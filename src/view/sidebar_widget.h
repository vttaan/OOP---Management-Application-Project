#ifndef SIDEBAR_WIDGET_H
#define SIDEBAR_WIDGET_H

#include "global.h"

namespace Ui
{
    class Sidebar_Widget;
}

class SessionManager;

class Sidebar_Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Sidebar_Widget(QWidget *parent = nullptr);
    ~Sidebar_Widget();
    QString getNormalStyle() { return this->normalStyle; }
    QString getActiveStyle() { return this->activeStyle; }
    void loadUserData(SessionManager *session);

signals:
    void menuClicked(int pageIndex);
    void logoutClicked();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupSidebarAvatar(const QString &imagePath);
    Ui::Sidebar_Widget *ui;
    // mainIndex is tab in sidebar, subIndex is tab in Schedule
    void updateButtonStyles(int mainIndex);
    void initUI();
    // CSS for normal (inactive) button — dark sidebar theme
    const QString normalStyle = "QPushButton { "
                                "   text-align: left; "
                                "   padding-left: 16px; "
                                "   font-size: 13px; "
                                "   font-weight: 500; "
                                "   color: #94A3B8; "
                                "   background-color: transparent; "
                                "   border: none; "
                                "   border-radius: 8px; "
                                "   height: 44px; "
                                "   margin: 2px 12px; "
                                "} "
                                "QPushButton:hover { "
                                "   background-color: #1E293B; "
                                "   color: #CBD5E1; "
                                "}";
    // CSS for clicked button
    // const QString logOutStyle = "QPushButton { "
    //                             "   text-align: left; "
    //                             "   padding-left: 35px; "
    //                             "   font-size: 14px; "
    //                             "   font-weight: 600; "
    //                             "   color: #475467; "
    //                             "   background-color: transparent; "
    //                             "   border: none; "
    //                             "   border-radius: 8px; "
    //                             "   height: 45px; "
    //                             "   margin: 4px 16px; "
    //                             "} "
    //                             "QPushButton:hover { "
    //                             "   background-color: #F62440; "
    //                             "}";
    const QString activeStyle =
        "QPushButton { "
        "   text-align: left; "
        "   padding-left: 16px; "
        "   font-size: 13px; "
        "   font-weight: bold; "
        "   color: #FFFFFF; "
        "   background-color: #2563EB; "
        "   border: none; "
        "   border-radius: 8px; "
        "   height: 44px; "
        "   margin: 2px 12px; "
        "}";
};

#endif