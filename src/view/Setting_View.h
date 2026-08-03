#ifndef SETTING_VIEW_H
#define SETTING_VIEW_H
#include "global.h"
#include <QWidget>

namespace Ui {
class Setting_View;
}

class Setting_View : public QWidget
{
    Q_OBJECT

public:
    explicit Setting_View(QWidget *parent = nullptr);
    ~Setting_View();
    void loadData(short openHour, short closeHour, Qt::DayOfWeek dayOpenRegis,
                      const QMap<QString, QPair<short, short>>& roles, short maxLeaveFT,
                      short maxDaysPT, short maxHourPT);

signals:
    void requestSave(short openHour, short closeHour, Qt::DayOfWeek dayOpenRegis,
                     const QMap<QString, QPair<short, short>>& roles, short maxLeaveFT,
                     short maxDaysPT, short maxHourPT);
    void requestCancel();

private slots:
    void saveClicked();

private:
    Ui::Setting_View *ui;
    void setupComboBox();
    void setupSpinBox();
    void setupTable();
    void initUI();
};

#endif // SETTING_VIEW_H
