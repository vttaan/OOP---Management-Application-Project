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
    void setPermission(const bool &permitted);
    void updateButtonStyles(int mainIndex);

signals:
    void menuClicked(int pageIndex);
    void logoutClicked();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupSidebarAvatar(const QString &imagePath);
    Ui::Sidebar_Widget *ui;
    // mainIndex is tab in sidebar, subIndex is tab in Schedule
    void initUI();
    // CSS for normal (inactive) button — glassmorphic cyan theme
    const QString normalStyle = "QPushButton { "
                                "   text-align: left; "
                                "   padding-left: 16px; "
                                "   font-size: 15px; "
                                "   font-weight: 600; "
                                "   color: rgba(255, 255, 255, 0.85); "
                                "   background-color: transparent; "
                                "   border: none; "
                                "   border-radius: 12px; "
                                "   height: 48px; "
                                "   margin: 4px 12px; "
                                "} "
                                "QPushButton:hover { "
                                "   background-color: rgba(255, 255, 255, 0.15); "
                                "   color: #FFFFFF; "
                                "}";
    // CSS for logout button — glassmorphic with red hover
    const QString logOutStyle = "QPushButton { "
                                "   text-align: left; "
                                "   padding-left: 16px; "
                                "   font-size: 15px; "
                                "   font-weight: 600; "
                                "   color: rgba(255, 255, 255, 0.85); "
                                "   background-color: transparent; "
                                "   border: none; "
                                "   border-radius: 12px; "
                                "   height: 48px; "
                                "   margin: 4px 12px; "
                                "} "
                                "QPushButton:hover { "
                                "   background-color: rgba(239, 68, 68, 0.8); "
                                "   color: #FFFFFF; "
                                "}";
    // CSS for active (selected) button — glassmorphic highlight
    const QString activeStyle =
        "QPushButton { "
        "   text-align: left; "
        "   padding-left: 16px; "
        "   font-size: 15px; "
        "   font-weight: bold; "
        "   color: #FFFFFF; "
        "   background-color: rgba(255, 255, 255, 0.25); "
        "   border: 1px solid rgba(255, 255, 255, 0.4); "
        "   border-radius: 12px; "
        "   height: 48px; "
        "   margin: 4px 12px; "
        "}";
};

#endif