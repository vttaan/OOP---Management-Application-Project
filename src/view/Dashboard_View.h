#ifndef DASHBOARD_VIEW_H
#define DASHBOARD_VIEW_H

#include <QWidget>
#include <QMouseEvent>
#include <QHBoxLayout>

class Dashboard_Control;

namespace Ui
{
class Dashboard_View;
}

class Dashboard_View : public QWidget
{
    Q_OBJECT

public:
    // Khai báo chuẩn khớp với kiến trúc nhóm
    explicit Dashboard_View(Dashboard_Control *controller = nullptr, QWidget *parent = nullptr);
    ~Dashboard_View();

    Dashboard_Control *getController() const;
    void setController(Dashboard_Control *controller);
    void embedHRPage(QWidget *hrWidget);
    void showHRPage();
    void embedWidgetIntoPage(int index, QWidget* widget);

    void switchPage(int index); // Hàm lật trang và đổi màu nút

public slots:
    void toggleSidebar(); // BẮT BUỘC phải là public slot để kết nối với nút Hamburger

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_btnMenu_Overview_clicked();
    void on_btnMenu_HR_clicked();
    void on_btnMenu_Timekeep_clicked();
    void on_btnMenu_Salary_clicked();
    void on_btnMenu_Report_clicked();
    void on_btnMenu_Settings_clicked();
    void on_btnLogout_clicked();

private:
    Ui::Dashboard_View *ui;
    Dashboard_Control *controller;
};

class clickableBox : public QWidget
{
    Q_OBJECT
private:
    QHBoxLayout *layout;

public:
    explicit clickableBox(QWidget *parent = nullptr) : QWidget(parent)
    {
        layout = new QHBoxLayout(this);
    }
    QHBoxLayout *getLayout() { return layout; }
signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        emit clicked();
        QWidget::mousePressEvent(event);
    }
};

#endif // DASHBOARD_VIEW_H