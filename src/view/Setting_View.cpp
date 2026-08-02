#include "Setting_View.h"
#include "ui_Setting_View.h"

Setting_View::Setting_View(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Setting_View)
{
    ui->setupUi(this);
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
    ui->tableRoles->verticalHeader()->setVisible(false);
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
    ui->tableRoles->setRowCount(0); // clear before load data
    int row = 0;


    for (auto it = roles.constBegin(); it != roles.constEnd(); ++it) {
        ui->tableRoles->insertRow(row);

        // col 1: Roles READ ONLY
        QTableWidgetItem* roleItem = new QTableWidgetItem(it.key());
        roleItem->setFlags(roleItem->flags() ^ Qt::ItemIsEditable);
        ui->tableRoles->setItem(row, 0, roleItem);

        // col 2:  Min Staff
        QSpinBox* spinMin = new QSpinBox();
        spinMin->setMinimum(0);
        spinMin->setMaximum(999);
        spinMin->setValue(it.value().first);
        ui->tableRoles->setCellWidget(row, 1, spinMin);

        // col 3: Max Staff
        QSpinBox* spinMax = new QSpinBox();
        spinMax->setMinimum(0);
        spinMax->setMaximum(999);
        spinMax->setValue(it.value().second);
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
