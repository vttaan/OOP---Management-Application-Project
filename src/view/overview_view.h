#ifndef OVERVIEW_VIEW_H
#define OVERVIEW_VIEW_H

#include <QWidget>
#include <QGridLayout>

namespace Ui {
class Overview_view;
}

class Overview_View : public QWidget
{
    Q_OBJECT

public:
    explicit Overview_View(QWidget *parent = nullptr);
    ~Overview_View();

    void clearEmployeeCards();
    void addEmployeeCard(const QString& avatarPath, const QString& name, const QString& role, const QString& email, const QString& phone);

    void clearSidePanels();
    void addNextShiftItem(const QString& name, const QString& timeInfo, const QString& colorHex = "#d9d9d9");
    void addOffEmployeeItem(const QString& name, const QString& reason, const QString& colorHex = "#ff4d4f");

signals:
    void toggleSidebarRequested();

private:
    Ui::Overview_view *ui;
    QGridLayout* gridLayoutEmployees;
    int currentRow;
    int currentCol;
};

#endif