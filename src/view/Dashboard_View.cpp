#include "Dashboard_View.h"
#include "ui_Dashboard_View.h"
#include "control/Dashboard_Control.h"
#include "employeecard.h"
#include <QGridLayout>
#include <QMouseEvent>

Dashboard_View::Dashboard_View(Dashboard_Control* controller, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Dashboard_View()),
    controller(controller)
{
    ui->setupUi(this);

    // 1. Tạo một Layout dạng Lưới (Grid) và gắn nó vào khu vực thanh cuộn
    QGridLayout* gridLayout = new QGridLayout(ui->scrollAreaWidgetContents);
    gridLayout->setSpacing(20); // Khoảng cách giữa các thẻ

    QStringList avatars = {":/images/image1.jpg", ":/images/image2.jpg", ":/images/image3.jpg",
                           ":/images/image4.jpg", ":/images/image5.jpg", ":/images/image6.jpg", ":/images/image7.jpg"};

    QStringList names = {"Nhân viên A", "Nhân viên B", "Nhân viên C", "Nhân viên D", "Nhân viên E", "Quản lý 1", "Quản lý 2"};
    QStringList roles = {"Nhân viên", "Nhân viên", "Nhân viên", "Nhân viên", "Nhân viên", "Quản lý", "Quản lý"};
    QStringList emails = {"a@congty.com", "b@congty.com", "c@congty.com", "d@congty.com", "e@congty.com", "ql1@congty.com", "ql2@congty.com"};
    QStringList phones = {"0901 111 111", "0902 222 222", "0903 333 333", "0904 444 444", "0905 555 555", "0906 666 666", "0907 777 777"};

    int row = 0;
    int col = 0;
    int maxColumns = 4;

    for(int i = 0; i < 7; i++) {
        EmployeeCard* card = new EmployeeCard(this);

        card->setData(avatars[i], names[i], roles[i], "✉️ " + emails[i], "📞 " + phones[i]);

        gridLayout->addWidget(card, row, col);

        col++;
        if (col >= maxColumns) {
            col = 0;
            row++;
        }
    }

    connect(ui->btnMenu_Overview, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(0); // Lật về trang Tổng quan (Giao diện cũ)
    });

    connect(ui->btnMenu_HR, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(1); // Lật sang trang trống Nhân sự
    });

    connect(ui->btnMenu_Timekeep, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(2); // Lật sang trang trống Chấm công
    });

    connect(ui->btnMenu_Salary, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(3); // Lật sang trang trống Lương
    });

    connect(ui->btnMenu_Report, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(4); // Lật sang trang trống Báo cáo
    });

    connect(ui->btnMenu_Settings, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(5); // Lật sang trang trống Cài đặt
    });


    ui->lblAvatar->setCursor(Qt::PointingHandCursor);
    ui->lblUserName->setCursor(Qt::PointingHandCursor);
    ui->lblUserRole->setCursor(Qt::PointingHandCursor);
    ui->lblDropdown->setCursor(Qt::PointingHandCursor);

    ui->lblAvatar->installEventFilter(this);
    ui->lblUserName->installEventFilter(this);
    ui->lblUserRole->installEventFilter(this);
    ui->lblDropdown->installEventFilter(this);


    // profile avatar dropbox
    QWidget* avatarBox = new QWidget(this);
    avatarBox->setLayout(ui->horizontalLayout_UserInfo);

}


Dashboard_View::~Dashboard_View()
{
    ui = nullptr;
    delete ui;
}

Dashboard_Control* Dashboard_View::getController() const {
    return controller;
}

bool Dashboard_View::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (watched == ui->lblAvatar || watched == ui->lblUserName ||
            watched == ui->lblUserRole || watched == ui->lblDropdown) {
            ui->stackedWidget->setCurrentIndex(0);
            emit this->getController()->profilePageClicked(); // switch to profile_view
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}
<<<<<<< Updated upstream
void Dashboard_View::on_btnLogout_clicked()
{
    emit this->getController()->logoutSubmitted();
=======

void Dashboard_View::on_btnMenu_Overview_clicked() {
    if (ui && ui->stackedWidget) ui->stackedWidget->setCurrentWidget(ui->pageOverview);
}

void Dashboard_View::on_btnMenu_HR_clicked() {
    if (ui && ui->stackedWidget) ui->stackedWidget->setCurrentWidget(ui->pageHR);
}

void Dashboard_View::on_btnMenu_Timekeep_clicked() {
    if (ui && ui->stackedWidget) ui->stackedWidget->setCurrentWidget(ui->pageTimekeep);
}

void Dashboard_View::on_btnMenu_Salary_clicked() {
    if (ui && ui->stackedWidget) ui->stackedWidget->setCurrentWidget(ui->pageSalary);
}

void Dashboard_View::on_btnMenu_Report_clicked() {
    if (ui && ui->stackedWidget) ui->stackedWidget->setCurrentWidget(ui->pageReport);
}

void Dashboard_View::on_btnMenu_Settings_clicked() {
    if (ui && ui->stackedWidget) ui->stackedWidget->setCurrentWidget(ui->pageSettings);
}

void Dashboard_View::setController(Dashboard_Control *controller) {
    this->controller = controller;
}

void Dashboard_View::on_btnLogout_clicked()
{
    if (controller) {
        qDebug() << "emit logout";
        emit controller->logoutSubmitted();
    }
>>>>>>> Stashed changes
}


