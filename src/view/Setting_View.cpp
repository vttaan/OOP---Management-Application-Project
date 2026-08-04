#include "Setting_View.h"
#include "ui_Setting_View.h"

Setting_View::Setting_View(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Setting_View)
{
    ui->setupUi(this);
    initUI();
    setupComboBox();
    setupSpinBox();
    setupTable();


    connect(ui->buttonSave, &QPushButton::clicked, this, &Setting_View::saveClicked);
    connect(ui->buttonCancel, &QPushButton::clicked, this, &Setting_View::requestCancel);
}

Setting_View::~Setting_View()
{
    delete ui;
}

void Setting_View::setupComboBox()
{
    ui->comboBoxDateRegis->addItem("Thứ Hai (Monday)", Qt::Monday);
    ui->comboBoxDateRegis->addItem("Thứ Ba (Tuesday)", Qt::Tuesday);
    ui->comboBoxDateRegis->addItem("Thứ Tư (Wednesday)", Qt::Wednesday);
    ui->comboBoxDateRegis->addItem("Thứ Năm (Thursday)", Qt::Thursday);
    ui->comboBoxDateRegis->addItem("Thứ Sáu (Friday)", Qt::Friday);
    ui->comboBoxDateRegis->addItem("Thứ Bảy (Saturday)", Qt::Saturday);
    ui->comboBoxDateRegis->addItem("Chủ Nhật (Sunday)", Qt::Sunday);

    ui->comboBoxFullTime->clear();
    ui->comboBoxFullTime->addItem("1 Ngày", 1);
    ui->comboBoxFullTime->addItem("2 Ngày", 2);
}

void Setting_View::setupSpinBox(){
    ui->spinBoxOpen->setRange(6, 23);
    ui->spinBoxClose->setRange(6, 23);
    ui->spinBoxDayMaxPartTime->setRange(1, 4);
    ui->spinBoxHourMaxPartTime->setRange(4, 7);
}

void Setting_View::setupTable()
{
    ui->tableRoles->setColumnCount(3);
    ui->tableRoles->setHorizontalHeaderLabels({"Tên Vị Trí", "Tối Thiểu (Min)", "Tối Đa (Max)"});

    ui->tableRoles->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableRoles->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->tableRoles->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->tableRoles->setColumnWidth(1, 130);
    ui->tableRoles->setColumnWidth(2, 130);
    ui->tableRoles->verticalHeader()->setVisible(false);
    ui->tableRoles->verticalHeader()->setDefaultSectionSize(32);
}

void Setting_View::loadData(short openHour, short closeHour, Qt::DayOfWeek dayOpenRegis,
                            const QMap<QString, QPair<short, short>>& roles, short maxLeaveFT,
                            short maxDaysPT, short maxHourPT)
{
    // load Date Time
    ui->spinBoxOpen->setValue(openHour);
    ui->spinBoxClose->setValue(closeHour);

    int idx = ui->comboBoxDateRegis->findData(dayOpenRegis);
    if(idx != -1) ui->comboBoxDateRegis->setCurrentIndex(idx);

    // load Date Full Time
    idx = ui->comboBoxFullTime->findData(maxLeaveFT);
    if(idx != -1) ui->comboBoxFullTime->setCurrentIndex(idx);

    // load Data Part Time
    ui->spinBoxDayMaxPartTime->setValue(maxDaysPT);
    ui->spinBoxHourMaxPartTime->setValue(maxHourPT);

    // load data to table roles
    ui->tableRoles->setRowCount(roles.size());
    int row = 0;

    for (auto it = roles.constBegin(); it != roles.constEnd(); ++it) {
        // col 1: Roles READ ONLY - bold + larger font
        QTableWidgetItem* roleItem = new QTableWidgetItem(it.key());
        roleItem->setFlags(roleItem->flags() ^ Qt::ItemIsEditable);
        QFont roleFont = roleItem->font();
        roleFont.setBold(true);
        roleFont.setPointSize(11);
        roleItem->setFont(roleFont);
        ui->tableRoles->setItem(row, 0, roleItem);

        // col 2: Min Staff - compact spinbox
        QSpinBox* spinMin = new QSpinBox(ui->tableRoles);
        spinMin->setMinimum(0);
        spinMin->setMaximum(999);
        spinMin->setValue(it.value().first);
        spinMin->setFixedHeight(32);
        spinMin->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
        ui->tableRoles->setCellWidget(row, 1, spinMin);

        // col 3: Max Staff - compact spinbox
        QSpinBox* spinMax = new QSpinBox(ui->tableRoles);
        spinMax->setMinimum(0);
        spinMax->setMaximum(999);
        spinMax->setValue(it.value().second);
        spinMax->setFixedHeight(32);
        spinMax->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
        ui->tableRoles->setCellWidget(row, 2, spinMax);

        row++;
    }
}

void Setting_View::saveClicked()
{

    short openHour = ui->spinBoxOpen->value();
    short closeHour = ui->spinBoxClose->value();
    Qt::DayOfWeek dayOpen = static_cast<Qt::DayOfWeek>(ui->comboBoxDateRegis->currentData().toInt());

    short maxLeaveFT = ui->comboBoxFullTime->currentData().toInt();

    short maxDaysPT = ui->spinBoxDayMaxPartTime->value();
    short maxHourPT = ui->spinBoxHourMaxPartTime->value();

    QMap<QString, QPair<short, short>> rolesMap;
    for (int i = 0; i < ui->tableRoles->rowCount(); ++i) {
        // Lấy tên chức danh từ Cột 0
        QString roleName = ui->tableRoles->item(i, 0)->text();

        // Ép kiểu (cast) widget trong ô ra thành QSpinBox để lấy giá trị
        QSpinBox* spinMin = qobject_cast<QSpinBox*>(ui->tableRoles->cellWidget(i, 1));
        QSpinBox* spinMax = qobject_cast<QSpinBox*>(ui->tableRoles->cellWidget(i, 2));

        if(spinMin && spinMax) {
            rolesMap[roleName] = qMakePair(static_cast<short>(spinMin->value()),
                                           static_cast<short>(spinMax->value()));
        }
    }

    emit requestSave(openHour, closeHour, dayOpen,
                     rolesMap, maxLeaveFT,
                     maxDaysPT, maxHourPT);
}

void Setting_View::initUI()
{
    this->setStyleSheet(
        "QWidget#Setting_View {"
        "   background-color: #F3F4F6;"
        "}"
        "QGroupBox {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E5E7EB;"
        "   border-radius: 12px;"
        "   margin-top: 24px;"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "   color: #1F2937;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top left;"
        "   left: 16px;"
        "   padding: 0 4px;"
        "   color: #1a73e8;"
        "}"
        "QLabel {"
        "   color: #374151;"
        "   font-size: 13px;"
        "   font-weight: bold;"
        "}"
        "QSpinBox, QComboBox, QLineEdit {"
        "   background-color: #F9FAFB;"
        "   border: 1px solid #D1D5DB;"
        "   border-radius: 6px;"
        "   padding: 4px 8px;"
        "   min-height: 24px;"
        "   color: #1F2937;"
        "}"
        "QSpinBox::up-button {"
        "   subcontrol-origin: border;"
        "   subcontrol-position: top right;"
        "   width: 24px;"
        "   background-color: #1a73e8;"
        "   border-top-right-radius: 5px;"
        "}"
        "QSpinBox::down-button {"
        "   subcontrol-origin: border;"
        "   subcontrol-position: bottom right;"
        "   width: 24px;"
        "   background-color: #1a73e8;"
        "   border-bottom-right-radius: 5px;"
        "}"
        "QSpinBox::up-button:hover, QSpinBox::down-button:hover {"
        "   background-color: #1558d6;"
        "}"
        "QSpinBox::up-button:pressed, QSpinBox::down-button:pressed {"
        "   background-color: #174ea6;"
        "}"
        "QSpinBox::up-arrow {"
        "   image: url(:/images/up-arrow.svg);"
        "   width: 12px; height: 12px;"
        "}"
        "QSpinBox::down-arrow {"
        "   image: url(:/images/down-arrow.svg);"
        "   width: 12px; height: 12px;"
        "}"
        "QComboBox::drop-down {"
        "   subcontrol-origin: padding;"
        "   subcontrol-position: top right;"
        "   width: 24px;"
        "   background-color: #1a73e8;"
        "   border-top-right-radius: 5px;"
        "   border-bottom-right-radius: 5px;"
        "}"
        "QComboBox::drop-down:hover {"
        "   background-color: #1558d6;"
        "}"
        "QComboBox::down-arrow {"
        "   image: url(:/images/down-arrow.svg);"
        "   width: 12px; height: 12px;"
        "}"
        "QTableWidget {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E5E7EB;"
        "   border-radius: 8px;"
        "   gridline-color: #F3F4F6;"
        "   color: #1F2937;"
        "}"
        "QHeaderView::section {"
        "   background-color: #F9FAFB;"
        "   color: #4B5563;"
        "   font-weight: bold;"
        "   border: none;"
        "   border-bottom: 1px solid #E5E7EB;"
        "   padding: 6px;"
        "}"
        "QPushButton {"
        "   background-color: #1a73e8;"
        "   color: white;"
        "   border-radius: 6px;"
        "   padding: 8px 16px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #1558d6;"
        "}"
        "QPushButton#buttonCancel {"
        "   background-color: #FFFFFF;"
        "   color: #374151;"
        "   border: 1px solid #D1D5DB;"
        "}"
        "QPushButton#buttonCancel:hover {"
        "   background-color: #F3F4F6;"
        "}"
    );
}
