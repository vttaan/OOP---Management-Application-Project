#include "sidebar_widget.h"
#include "ui_sidebar_widget.h"

Sidebar_Widget::Sidebar_Widget(QWidget *parent) :
    QWidget(parent), ui(new Ui::Sidebar_Widget)
{
    ui->setupUi(this);
    initUI();
    ui->subMenu_Schedule->hide(); // setup in default, when start program

    // logic in Schedule tab, because Schedule tab has 3 subTab.
    connect(ui->buttonSchedule, &QPushButton::clicked, [this]() {
        bool isHidden = ui->subMenu_Schedule->isHidden();
        ui->subMenu_Schedule->setVisible(isHidden);
    });

    // main tab
    connect(ui->btnMenu_Overview, &QPushButton::clicked, [this]() { emit menuClicked(1); updateButtonStyles(1); });
    connect(ui->btnMenu_HR, &QPushButton::clicked, [this]() { emit menuClicked(3); updateButtonStyles(3); });
    //connect(ui->btnMenu_Salary, &QPushButton::clicked, [this]() { emit menuClicked(7); updateButtonStyles(7); });
    //connect(ui->btnMenu_Report, &QPushButton::clicked, [this]() { emit menuClicked(8); updateButtonStyles(8); });
    //connect(ui->btnMenu_Settings, &QPushButton::clicked, [this]() { emit menuClicked(9); updateButtonStyles(9); });

    // subTab in Schedule
    connect(ui->buttonRegistrationSchedule, &QPushButton::clicked, [this]() { emit menuClicked(4); updateButtonStyles(4); });
    connect(ui->buttonArrangeSchedule, &QPushButton::clicked, [this]() { emit menuClicked(5); updateButtonStyles(5); });
    connect(ui->buttonViewSchedule, &QPushButton::clicked, [this]() { emit menuClicked(6); updateButtonStyles(6); });

    connect(ui->btnLogout, &QPushButton::clicked, [this]() { emit logoutClicked(); });

    // default set view in dashboard
    updateButtonStyles(1);
}

Sidebar_Widget::~Sidebar_Widget() { delete ui; }

void Sidebar_Widget::initUI() {

    // setup background
    this->setStyleSheet("QWidget#Sidebar_Widget { "
                        "   background-color: #FFFFFF; "
                        "   border-right: 1px solid #E5E7EB; "
                        "}");

    // setup logo
    if (ui->labelLogo) {
        ui->labelLogo->setText("Hệ thống quản lý\nnhân sự");
        ui->labelLogo->setAlignment(Qt::AlignCenter);
        ui->labelLogo->setStyleSheet(
            "font-size: 16px; "
            "font-weight: 900; "
            "color: #111827; "
            "border: none; "
            "margin-top: 24px; "
            "margin-bottom: 32px;"
            );
    }

    // setup default
    updateButtonStyles(1);
}

void Sidebar_Widget::updateButtonStyles(int mainIndex)
{

    // CSS / AI GEN
    QString activeMain = this->getActiveStyle();
    QString activeSub = activeMain;
    activeSub.replace("padding-left: 35px;", "padding-left: 55px;");
    QString normal = this->getNormalStyle();
    QString normalSub = normal;
    normalSub.replace("padding-left: 35px;", "padding-left: 55px;");
    // Reset all
    ui->btnMenu_Overview->setStyleSheet(normal);
    ui->btnMenu_HR->setStyleSheet(normal);
    ui->buttonSchedule->setStyleSheet(normal);
    ui->btnMenu_Salary->setStyleSheet(normal);
    ui->btnMenu_Report->setStyleSheet(normal);
    ui->btnMenu_Settings->setStyleSheet(normal);
    ui->buttonRegistrationSchedule->setStyleSheet(normal);
    ui->buttonArrangeSchedule->setStyleSheet(normal);
    ui->buttonViewSchedule->setStyleSheet(normal);

    switch(mainIndex) {
    case 1: ui->btnMenu_Overview->setStyleSheet(activeMain); break;
    case 3: ui->btnMenu_HR->setStyleSheet(activeMain); break;
    case 4:
        ui->buttonSchedule->setStyleSheet(activeMain);
        ui->buttonRegistrationSchedule->setStyleSheet(activeSub);
        break;
    case 5:
        ui->buttonSchedule->setStyleSheet(activeMain);
        ui->buttonArrangeSchedule->setStyleSheet(activeSub);
        break;
    case 6:
        ui->buttonSchedule->setStyleSheet(activeMain);
        ui->buttonViewSchedule->setStyleSheet(activeSub);
        break;
    }
}