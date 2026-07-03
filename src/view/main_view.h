#ifndef MAIN_VIEW_H
#define MAIN_VIEW_H
#include <QEvent>
#include <QWidget>
#include <QHBoxLayout>

namespace Ui
{
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

private:
    Ui::Main_View *ui;
signals:
    void logoutSubmitted();
    void profilePageClicked();
};

class clickableBox : public QWidget
{
    Q_OBJECT
private:
    QHBoxLayout *layout;

public:
    clickableBox(QWidget *parent = nullptr) : QWidget(parent)
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
    }
};

#endif