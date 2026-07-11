#include "sidebar_widget.h"
#include "ui_sidebar_widget.h"

Sidebar_Widget::Sidebar_Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Sidebar_Widget)
{
    ui->setupUi(this);

    updateButtonStyles(0);

    // Bắt sự kiện Click và phát loa ra ngoài + Tự đổi màu
    connect(ui->btnMenu_Overview, &QPushButton::clicked, [this]() {
        emit menuClicked(0);
        updateButtonStyles(0);
    });
    connect(ui->btnMenu_HR, &QPushButton::clicked, [this]() {
        emit menuClicked(1);
        updateButtonStyles(1);
    });
    connect(ui->btnMenu_Timekeep, &QPushButton::clicked, [this]() {
        emit menuClicked(2);
        updateButtonStyles(2);
    });
    connect(ui->btnMenu_Salary, &QPushButton::clicked, [this]() {
        emit menuClicked(3);
        updateButtonStyles(3);
    });
    connect(ui->btnMenu_Report, &QPushButton::clicked, [this]() {
        emit menuClicked(4);
        updateButtonStyles(4);
    });
    connect(ui->btnMenu_Settings, &QPushButton::clicked, [this]() {
        emit menuClicked(5);
        updateButtonStyles(5);
    });

    // Nút Đăng xuất
    connect(ui->btnLogout, &QPushButton::clicked, [this]() {
        emit logoutClicked();
    });
}

Sidebar_Widget::~Sidebar_Widget()
{
    delete ui;
}

void Sidebar_Widget::updateButtonStyles(int activeIndex)
{
    QString defaultStyle = "QPushButton { text-align: left; padding-left: 20px; font-size: 15px; font-weight: 500; color: #637381; border: none; border-radius: 8px; height: 45px; margin: 5px 15px; } "
                           "QPushButton:hover { background-color: #f4f6f8; color: #212b36; }";

    QString activeStyle = "QPushButton { text-align: left; padding-left: 20px; font-size: 15px; font-weight: bold; color: #1a73e8; background-color: #e8f0fe; border: none; border-radius: 8px; height: 45px; margin: 5px 15px; }";

    ui->btnMenu_Overview->setStyleSheet(defaultStyle);
    ui->btnMenu_HR->setStyleSheet(defaultStyle);
    ui->btnMenu_Timekeep->setStyleSheet(defaultStyle);
    ui->btnMenu_Salary->setStyleSheet(defaultStyle);
    ui->btnMenu_Report->setStyleSheet(defaultStyle);
    ui->btnMenu_Settings->setStyleSheet(defaultStyle);

    switch(activeIndex) {
    case 0: ui->btnMenu_Overview->setStyleSheet(activeStyle); break;
    case 1: ui->btnMenu_HR->setStyleSheet(activeStyle); break;
    case 2: ui->btnMenu_Timekeep->setStyleSheet(activeStyle); break;
    case 3: ui->btnMenu_Salary->setStyleSheet(activeStyle); break;
    case 4: ui->btnMenu_Report->setStyleSheet(activeStyle); break;
    case 5: ui->btnMenu_Settings->setStyleSheet(activeStyle); break;
    }
}