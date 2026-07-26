#include "Overview_view.h"
#include "ui_overview_view.h"
#include "employeecard.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>

Overview_View::Overview_View(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Overview_view())
{
    ui->setupUi(this);

    // Đã sửa chữ 'v' thường thành 'V' hoa cho khớp với tên Class
    connect(ui->btnToggleSidebar, &QPushButton::clicked, this, &Overview_View::toggleSidebarRequested);

    // Khởi tạo lưới trống bên trong trang tổng quan mới
    gridLayoutEmployees = new QGridLayout(ui->scrollAreaWidgetContents);
    gridLayoutEmployees->setSpacing(20);
    currentRow = 0;
    currentCol = 0;
}

Overview_View::~Overview_View()
{
    delete ui;
}

void Overview_View::clearEmployeeCards()
{
    QLayoutItem *child;
    while ((child = gridLayoutEmployees->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    currentRow = 0;
    currentCol = 0;
}

void Overview_View::addEmployeeCard(const QString& avatarPath, const QString& name, const QString& role, const QString& email, const QString& phone)
{
    EmployeeCard* card = new EmployeeCard(this);
    card->setData(avatarPath, name, role, email, phone);

    gridLayoutEmployees->addWidget(card, currentRow, currentCol);
    currentCol++;
    if (currentCol >= 4) {
        currentCol = 0;
        currentRow++;
    }
}

void Overview_View::clearSidePanels()
{
    ui->listNextShift->clear();
    ui->listOffEmployees->clear();
}

void Overview_View::addNextShiftItem(const QString& name, const QString& timeInfo, const QString& colorHex)
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

void Overview_View::addOffEmployeeItem(const QString& name, const QString& reason, const QString& colorHex)
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