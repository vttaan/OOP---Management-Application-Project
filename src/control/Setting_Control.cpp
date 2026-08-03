#include "Setting_Control.h"

Setting_Control::Setting_Control(QObject *parent)
    : QObject(parent), view(nullptr), model(new Setting_Model())
{
}

Setting_Control::~Setting_Control() {
    delete model;
}

void Setting_Control::setView(Setting_View* v) {
    if (v) view = v;
    else return;
    connect(view, &Setting_View::requestSave, this, &Setting_Control::handleSave);
    connect(view, &Setting_View::requestCancel, this, &Setting_Control::handleCancel);
}
void Setting_Control::init()
{
    short openHour = 0, closeHour = 0, maxLeaveFT = 0, maxDaysPT = 0, maxHourPT = 0;
    Qt::DayOfWeek dayOpenRegis = Qt::Monday;
    QMap<QString, QPair<short, short>> roles;

    if (model->loadData(openHour, closeHour, dayOpenRegis, roles, maxLeaveFT, maxDaysPT, maxHourPT)) {
        view->loadData(openHour, closeHour, dayOpenRegis, roles, maxLeaveFT, maxDaysPT, maxHourPT);
        qDebug() << "Init dữ liệu Setting thành công!";
    } else {
        qDebug() << "Lỗi: Không thể init dữ liệu từ Database.";
    }
}

void Setting_Control::handleSave(short openHour, short closeHour, Qt::DayOfWeek dayOpenRegis,
                                    QMap<QString, QPair<short, short>> roles,
                                    short maxLeaveFT, short maxDaysPT, short maxHourPT)
{
    if (model->saveData(openHour, closeHour, dayOpenRegis, roles, maxLeaveFT, maxDaysPT, maxHourPT)) {
        qDebug() << "Đã lưu cài đặt và cập nhật Config trên RAM thành công!";
        QMessageBox::information(view, "Thành công", "Đã lưu cài đặt hệ thống!");
    } else {
        qDebug() << "Lưu cài đặt thất bại!";
        QMessageBox::critical(view, "Lỗi", "Đã xảy ra lỗi khi lưu vào Database.");
    }
}

void Setting_Control::handleCancel()
{
    init();
    qDebug() << "Đã hủy các thay đổi và nạp lại cấu hình gốc.";
}