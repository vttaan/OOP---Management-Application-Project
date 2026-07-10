#include "global.h"
#ifndef MAIN_VIEW_H
#define MAIN_VIEW_H

namespace Ui {
class Main_View;
}

class Main_View : public QWidget
{
    Q_OBJECT

public:
    explicit Main_View(QWidget *parent = nullptr);
    ~Main_View();
    QWidget* getSidebar();
    void switchPage(int pageIndex);

    void clearEmployeeCards();
    void addEmployeeCard(const QString& avatarPath, const QString& name, const QString& role, const QString& email, const QString& phone);
    void clearSidePanels();
    void addNextShiftItem(const QString& name, const QString& timeInfo, const QString& colorHex = "#d9d9d9");
    void addOffEmployeeItem(const QString& name, const QString& reason, const QString& colorHex = "#ff4d4f");

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void menuOverviewClicked();
    void menuHRClicked();
    void menuTimekeepClicked();
    void menuSalaryClicked();
    void menuReportClicked();
    void menuSettingsClicked();
    void profileClicked();
    void toggleSidebarClicked();
private:
    Ui::Main_View *ui;

    QGridLayout* gridLayoutEmployees;
    int currentRow;
    int currentCol;
};

#endif