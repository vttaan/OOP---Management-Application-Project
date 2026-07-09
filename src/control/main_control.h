#ifndef MAIN_CONTROL_H
#define MAIN_CONTROL_H

#include <QObject>
#include "view/main_view.h"

class Main_Control : public QObject
{
    Q_OBJECT
public:
    explicit Main_Control(QObject *parent = nullptr);
    ~Main_Control();

    void init();
private:
    Main_View* view;
    bool isSidebarExpanded = true;
private slots:
    void handleOverviewClicked();
    void handleHRClicked();
    void handleTimekeepClicked();
    void handleSalaryClicked();
    void handleReportClicked();
    void handleSettingsClicked();
    void handleProfileClicked();
    void handleToggleSidebar();
};

#endif