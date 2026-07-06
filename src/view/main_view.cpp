#include "main_view.h"
#include "ui_main_view.h"
#include "employeecard.h"
#include <QHBoxLayout>
#include <QLabel>

Main_View::Main_View(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Main_View)
{
    ui->setupUi(this);

    gridLayoutEmployees = new QGridLayout(ui->scrollAreaWidgetContents);
    gridLayoutEmployees->setSpacing(20);
    currentRow = 0;
    currentCol = 0;

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

    QString defaultStyle = "QPushButton { text-align: left; padding-left: 20px; font-size: 15px; font-weight: 500; color: #637381; border: none; border-radius: 8px; height: 45px; margin: 5px 15px; } "
                           "QPushButton:hover { background-color: #f4f6f8; color: #212b36; }";

    QString activeStyle = "QPushButton { text-align: left; padding-left: 20px; font-size: 15px; font-weight: bold; color: #1a73e8; background-color: #e8f0fe; border: none; border-radius: 8px; height: 45px; margin: 5px 15px; }";

    ui->btnMenu_Overview->setStyleSheet(defaultStyle);
    ui->btnMenu_HR->setStyleSheet(defaultStyle);
    ui->btnMenu_Timekeep->setStyleSheet(defaultStyle);
    ui->btnMenu_Salary->setStyleSheet(defaultStyle);
    ui->btnMenu_Report->setStyleSheet(defaultStyle);
    ui->btnMenu_Settings->setStyleSheet(defaultStyle);

    switch(pageIndex) {
    case 0: ui->btnMenu_Overview->setStyleSheet(activeStyle); break;
    case 1: ui->btnMenu_HR->setStyleSheet(activeStyle); break;
    case 2: ui->btnMenu_Timekeep->setStyleSheet(activeStyle); break;
    case 3: ui->btnMenu_Salary->setStyleSheet(activeStyle); break;
    case 4: ui->btnMenu_Report->setStyleSheet(activeStyle); break;
    case 5: ui->btnMenu_Settings->setStyleSheet(activeStyle); break;
    }
}

bool Main_View::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (watched == ui->lblAvatar || watched == ui->lblUserName ||
            watched == ui->lblUserRole || watched == ui->lblDropdown) {

            emit profileClicked();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void Main_View::clearEmployeeCards()
{
    QLayoutItem *child;
    while ((child = gridLayoutEmployees->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    currentRow = 0;
    currentCol = 0;
}

void Main_View::addEmployeeCard(const QString& avatarPath, const QString& name, const QString& role, const QString& email, const QString& phone)
{
    EmployeeCard* card = new EmployeeCard(this);
    card->setData(avatarPath, name, role, email, phone);

    gridLayoutEmployees->addWidget(card, currentRow, currentCol);
    currentCol++;

    if (currentCol >= 3) {
        currentCol = 0;
        currentRow++;
    }
}


void Main_View::clearSidePanels()
{
    ui->listNextShift->clear();
    ui->listOffEmployees->clear();
}

void Main_View::addNextShiftItem(const QString& name, const QString& timeInfo, const QString& colorHex)
{
    QListWidgetItem* item = new QListWidgetItem(ui->listNextShift);

    QWidget* rowWidget = new QWidget();
    rowWidget->setStyleSheet("background: transparent;");

    QHBoxLayout* layout = new QHBoxLayout(rowWidget);
    layout->setContentsMargins(10, 5, 10, 5);
    layout->setSpacing(12);
    QLabel* dot = new QLabel();
    dot->setFixedSize(20, 20);
    dot->setStyleSheet(QString("QLabel { background-color: %1; border-radius: 10px; border: 1px solid %1; }").arg(colorHex));

    QLabel* lblInfo = new QLabel(QString("%1, (%2)").arg(name, timeInfo));
    lblInfo->setStyleSheet("QLabel { font-size: 14px; color: black; font-weight: 500; background: transparent; border: none; }");

    layout->addWidget(dot);
    layout->addWidget(lblInfo);
    layout->addStretch();

    item->setSizeHint(QSize(0, 45));
    ui->listNextShift->setItemWidget(item, rowWidget);
}

void Main_View::addOffEmployeeItem(const QString& name, const QString& reason, const QString& colorHex)
{
    QListWidgetItem* item = new QListWidgetItem(ui->listOffEmployees);

    QWidget* rowWidget = new QWidget();
    rowWidget->setStyleSheet("background: transparent;");

    QHBoxLayout* layout = new QHBoxLayout(rowWidget);
    layout->setContentsMargins(10, 5, 10, 5);
    layout->setSpacing(12);

    QLabel* dot = new QLabel();
    dot->setFixedSize(20, 20);
    dot->setStyleSheet(QString("QLabel { background-color: %1; border-radius: 10px; border: 1px solid %1; }").arg(colorHex));

    QLabel* lblInfo = new QLabel(QString("%1, (%2)").arg(name, reason));
    lblInfo->setStyleSheet("QLabel { font-size: 14px; color: black; font-weight: 500; background: transparent; border: none; }");

    layout->addWidget(dot);
    layout->addWidget(lblInfo);
    layout->addStretch();

    item->setSizeHint(QSize(0, 45));
    ui->listOffEmployees->setItemWidget(item, rowWidget);
}