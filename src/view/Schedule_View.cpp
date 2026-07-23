#include "global.h"
#include "Schedule_View.h"
#include "ui_Schedule_View.h"

Schedule_View::Schedule_View(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Schedule_View)
{
    ui->setupUi(this);
    setUpUI();

    connect(ui->ButtonThem, &QPushButton::clicked, this, &Schedule_View::buttonAddClicked);
    connect(ui->buttonLuu, &QPushButton::clicked, this, &Schedule_View::buttonSaveClicked);
}

Schedule_View::~Schedule_View()
{
    delete ui;
}

void Schedule_View::setUpUI() {

    ui->tableDangKy->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableSum->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    // Force uppercase for horizontal headers
    for(int i = 0; i < ui->tableDangKy->columnCount(); ++i) {
        if(ui->tableDangKy->horizontalHeaderItem(i)) {
            ui->tableDangKy->horizontalHeaderItem(i)->setText(ui->tableDangKy->horizontalHeaderItem(i)->text().toUpper());
        }
    }
    for(int i = 0; i < ui->tableSum->columnCount(); ++i) {
        if(ui->tableSum->horizontalHeaderItem(i)) {
            ui->tableSum->horizontalHeaderItem(i)->setText(ui->tableSum->horizontalHeaderItem(i)->text().toUpper());
        }
    }

    ui->tableDangKy->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableDangKy->setFocusPolicy(Qt::NoFocus);
    ui->tableSum->setFocusPolicy(Qt::NoFocus);
    // change can not adjust information inside table sum
    ui->tableSum->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // change can not select info
    //ui->tableSum->setSelectionMode(QAbstractItemView::NoSelection);
    this->setStyleSheet("background-color: #FFFFFF;");

    ui->ButtonThem->setStyleSheet(
        "QPushButton { background-color: #2F80ED; color: white; border-radius: 6px; padding: 6px 15px; font-weight: bold; } "
        "QPushButton:hover { background-color: #1C64F2; }"
        );

    ui->buttonLuu->setStyleSheet(
        "QPushButton { background-color: #219653; color: white; border-radius: 6px; padding: 8px 30px; font-weight: bold; font-size: 14px; } "
        "QPushButton:hover { background-color: #1E824C; }"
        );

    QString tableStyle =
        "QTableWidget { border: none; background-color: white; gridline-color: #EFEFEF; } "
        "QHeaderView::section { background-color: #F8F9FA; border: none; border-bottom: 2px solid #E5E7EB; padding: 8px; font-weight: bold; color: black; text-transform: uppercase; } "
        "QHeaderView::section:vertical { background-color: #E5E7EB; color: black; border: none; font-weight: bold; }";

    ui->tableDangKy->setStyleSheet(tableStyle);

    ui->tableSum->setStyleSheet(tableStyle);

}

QComboBox* Schedule_View::createComboBox(const QList<QString>& item) {
    QComboBox *cb = new QComboBox(this);
    cb->addItems(item);
    cb->setEditable(false);
    cb->setInsertPolicy(QComboBox::NoInsert);
    cb->setStyleSheet("QComboBox { padding: 4px; border: 1px solid #D0D5DD; border-radius: 4px; }");
    return cb;
}

void Schedule_View::setUpDataInputTable(const QList<QString>& listDays, int openTime, int closeTime) {
    QList<QString> listHour;
    QString postFix = ":00";
    for(int i = openTime; i <= closeTime; i++) listHour.append(QString::number(i) + postFix);

    ui->tableDangKy->setCellWidget(0, 0, createComboBox(listDays));
    ui->tableDangKy->setCellWidget(0, 1, createComboBox(listHour));
    ui->tableDangKy->setCellWidget(0, 2, createComboBox(listHour));
}

void Schedule_View::enableRegistration(bool isEnable) {
    ui->tableDangKy->setEnabled(isEnable);
    ui->ButtonThem->setEnabled(isEnable);
    ui->buttonLuu->setEnabled(isEnable);

    if(isEnable) {
        ui->DangKyLich->setText("ĐĂNG KÝ LỊCH LÀM");
        ui->DangKyLich->setStyleSheet("color: #333333; font-size: 16px; font-weight: bold;");
    }
    else {
        ui->DangKyLich->setText("CHƯA ĐẾN NGÀY ĐĂNG KÝ");
        ui->DangKyLich->setStyleSheet("color: #E02424; font-size: 16px; font-weight: bold;");
    }
}

void Schedule_View::updateSumTable(const QMap<int, QList<QString>> weeklyData) {
    int maxRow = 1;
    for(auto it = weeklyData.begin(); it != weeklyData.end(); it++) maxRow = qMax(maxRow, it.value().size());

    ui->tableSum->setRowCount(maxRow);
    ui->tableSum->clearContents();

    for(auto it = weeklyData.begin(); it != weeklyData.end(); it++) {
        QList<QString> shift = it.value();
        for(int j = 0; j < shift.size(); j++) {
            QTableWidgetItem* item = new QTableWidgetItem(shift[j]);
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableSum->setItem(j, it.key(), item);
        }
    }
}

void Schedule_View::showError(const QString& mess) {
    QMessageBox::warning(this, "Lỗi Trùng Giờ", mess);
}

void Schedule_View::buttonAddClicked() {
    QComboBox* day = qobject_cast<QComboBox*>(ui->tableDangKy->cellWidget(0, 0));
    QComboBox* startTime = qobject_cast<QComboBox*>(ui->tableDangKy->cellWidget(0, 1));
    QComboBox* endTime = qobject_cast<QComboBox*>(ui->tableDangKy->cellWidget(0, 2));

    if(day && startTime && endTime) emit requestAddShift(day->currentText(), startTime->currentText(), endTime->currentText());
}

void Schedule_View::buttonSaveClicked() {
    emit requestSaveShift();
}

void Schedule_View::resetInputTable() {
    QComboBox* day = qobject_cast<QComboBox*>(ui->tableDangKy->cellWidget(0, 0));
    QComboBox* startTime = qobject_cast<QComboBox*>(ui->tableDangKy->cellWidget(0, 1));
    QComboBox* endTime = qobject_cast<QComboBox*>(ui->tableDangKy->cellWidget(0, 2));
    if(day && startTime && endTime) {
        day->setCurrentIndex(0);
        startTime->setCurrentIndex(0);
        endTime->setCurrentIndex(0);
    }
}

