#include "main_view.h"
#include "ui_main_view.h"
#include "employeecard.h"
#include <QGridLayout>
#include <QMouseEvent>

Main_View::Main_View(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Main_View)
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
    connect(ui->btnMenu_Overview, &QPushButton::clicked, this, &Main_View::menuOverviewClicked);
    connect(ui->btnMenu_HR, &QPushButton::clicked, this, &Main_View::menuHRClicked);
    connect(ui->btnMenu_Timekeep, &QPushButton::clicked, this, &Main_View::menuTimekeepClicked);
    connect(ui->btnMenu_Salary, &QPushButton::clicked, this, &Main_View::menuSalaryClicked);
    connect(ui->btnMenu_Report, &QPushButton::clicked, this, &Main_View::menuReportClicked);
    connect(ui->btnMenu_Settings, &QPushButton::clicked, this, &Main_View::menuSettingsClicked);

    ui->lblAvatar->setCursor(Qt::PointingHandCursor);
    ui->lblUserName->setCursor(Qt::PointingHandCursor);
    ui->lblUserRole->setCursor(Qt::PointingHandCursor);
    ui->lblDropdown->setCursor(Qt::PointingHandCursor);

    ui->lblAvatar->installEventFilter(this);
    ui->lblUserName->installEventFilter(this);
    ui->lblUserRole->installEventFilter(this);
    ui->lblDropdown->installEventFilter(this);
}

Main_View::~Main_View()
{
    delete ui;
}

void Main_View::switchPage(int pageIndex)
{
    ui->stackedWidget->setCurrentIndex(pageIndex);
}

bool Main_View::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (watched == ui->lblAvatar || watched == ui->lblUserName ||
            watched == ui->lblUserRole || watched == ui->lblDropdown) {

            emit profileClicked();
        }
    }
    return QWidget::eventFilter(watched, event);
}