#ifndef MAIN_VIEW_H
#define MAIN_VIEW_H

#include <QWidget>
#include <QEvent>

namespace Ui {
class Main_View;
}

class Main_View : public QWidget
{
    Q_OBJECT

public:
    explicit Main_View(QWidget *parent = nullptr);
    ~Main_View();

    void switchPage(int pageIndex);

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

private:
    Ui::Main_View *ui;
};

#endif