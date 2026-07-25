#include "global.h"
#include "Schedule_View.h"
#include "ui_Schedule_View.h"

Schedule_View::Schedule_View(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Schedule_View)
{
    ui->setupUi(this);
    setUpUI();
    connect(ui->btnGenSchedule, &QPushButton::clicked, this, &Schedule_View::requestGenSchedule);
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

    ui->btnGenSchedule->setStyleSheet(
        "QPushButton { background-color: #9B51E0; color: white; border-radius: 6px; padding: 8px 30px; font-weight: bold; font-size: 14px; } "
        "QPushButton:hover { background-color: #8244C3; }"
        );

    ui->tableDangKy->setProperty("role", "staff");
    ui->tableSum->setProperty("role", "staff");

}

QComboBox* Schedule_View::createComboBox(const QList<QString>& item) {
    QComboBox *cb = new QComboBox(this);
    cb->addItems(item);
    cb->setEditable(false);
    cb->setInsertPolicy(QComboBox::NoInsert);
    cb->setStyleSheet("QComboBox { padding: 4px; border: 1px solid #D0D5DD; border-radius: 4px; }");
    return cb;
}

void Schedule_View::updateTableHeaders(QDate monday)
{
    QStringList headers;
    QStringList days = {"THỨ 2", "THỨ 3", "THỨ 4", "THỨ 5", "THỨ 6", "THỨ 7", "CHỦ NHẬT"};
    for (int i = 0; i < 7; ++i) {
        headers << QString("%1\n%2").arg(days[i], monday.addDays(i).toString("dd-MM-yyyy"));
    }
    ui->tableSum->setHorizontalHeaderLabels(headers);
}

void Schedule_View::setUpDataInputTable(QDate monday, int openTime, int closeTime) {
    QList<QString> listDaysWithDate;
    QStringList days = {"Thứ 2", "Thứ 3", "Thứ 4", "Thứ 5", "Thứ 6", "Thứ 7", "CN"};
    for (int i = 0; i < 7; ++i) {
        listDaysWithDate.append(QString("%1 (%2)").arg(days[i], monday.addDays(i).toString("dd-MM-yyyy")));
    }
    
    QList<QString> listHour;
    QString postFix = ":00";
    for(int i = openTime; i <= closeTime; i++) listHour.append(QString::number(i) + postFix);

    ui->tableDangKy->setCellWidget(0, 0, createComboBox(listDaysWithDate));
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
            item->setForeground(QBrush(Qt::black));
            ui->tableSum->setItem(j, it.key(), item);
        }
    }
}

void Schedule_View::showError(const QString& mess) {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Lỗi Trùng Giờ");
    msgBox.setText(mess);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStyleSheet("QMessageBox { background-color: #1e1e1e; } "
                         "QLabel { color: #ffffff; background-color: transparent; } "
                         "QPushButton { color: #ffffff; background-color: #333333; padding: 5px 15px; border-radius: 3px; }");
    msgBox.exec();
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
void Schedule_View::showSuccess(const QString& msg) {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Xếp lịch thành công");
    msgBox.setText(msg);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setStyleSheet("QMessageBox { background-color: #1e1e1e; } "
                         "QLabel { color: #ffffff; background-color: transparent; } "
                         "QPushButton { color: #ffffff; background-color: #333333; padding: 5px 15px; border-radius: 3px; }");
    msgBox.exec();
}
void Schedule_View::showWarnings(const QStringList& warnings) {
    QMessageBox dlg(this);
    dlg.setWindowTitle("Cảnh báo lịch làm việc");
    dlg.setIcon(QMessageBox::Warning);
    dlg.setText(QString("Xếp lịch hoàn tất nhưng có %1 cảnh báo:")
                    .arg(warnings.size()));
    dlg.setDetailedText(warnings.join("\n"));
    dlg.setStyleSheet("QMessageBox { background-color: #1e1e1e; } "
                      "QLabel { color: #ffffff; background-color: transparent; } "
                      "QPushButton { color: #ffffff; background-color: #333333; padding: 5px 15px; border-radius: 3px; } "
                      "QTextEdit { background-color: #2b2b2b; color: #ffffff; border: 1px solid #555; }");
    dlg.exec();
}

void Schedule_View::setManagerMode(bool isManager)
{
    if (isManager) {
        ui->DangKyLich->setVisible(false);
        ui->tableDangKy->setVisible(false);
        ui->ButtonThem->setVisible(false);
        ui->buttonLuu->setVisible(false);
        
        ui->btnGenSchedule->setVisible(true);
        ui->XacNhanLich->setText("<html><head/><body><p><span style=\" font-size:12pt; font-weight:700;\">THỐNG KÊ YÊU CẦU ĐĂNG KÝ TRONG TUẦN</span></p></body></html>");
        
        ui->tableSum->setRowCount(14);
        ui->tableSum->setColumnCount(7);
        QList<QString> timeSlots;
        for (int i = 8; i <= 21; ++i) {
            timeSlots << QString("%1:00 - %2:00").arg(i, 2, 10, QChar('0')).arg(i + 1, 2, 10, QChar('0'));
        }
        ui->tableSum->setVerticalHeaderLabels(timeSlots);
        ui->tableSum->verticalHeader()->setVisible(true);
        ui->tableSum->setSelectionMode(QAbstractItemView::NoSelection);
        
        ui->tableSum->setProperty("role", "manager");
        ui->tableSum->style()->unpolish(ui->tableSum);
        ui->tableSum->style()->polish(ui->tableSum);
        
        ui->tableSum->horizontalHeader()->style()->unpolish(ui->tableSum->horizontalHeader());
        ui->tableSum->horizontalHeader()->style()->polish(ui->tableSum->horizontalHeader());
        ui->tableSum->verticalHeader()->style()->unpolish(ui->tableSum->verticalHeader());
        ui->tableSum->verticalHeader()->style()->polish(ui->tableSum->verticalHeader());
    } else {
        ui->DangKyLich->setVisible(true);
        ui->tableDangKy->setVisible(true);
        ui->ButtonThem->setVisible(true);
        ui->buttonLuu->setVisible(true);
        
        ui->btnGenSchedule->setVisible(false);
        ui->XacNhanLich->setText("<html><head/><body><p><span style=\" font-size:12pt; font-weight:700;\">LỊCH ĐÃ ĐĂNG KÝ</span></p></body></html>");
        
        ui->tableSum->setColumnCount(7);
        ui->tableSum->verticalHeader()->setVisible(false);
        ui->tableSum->setSelectionMode(QAbstractItemView::SingleSelection);
        
        ui->tableSum->setProperty("role", "staff");
        ui->tableSum->style()->unpolish(ui->tableSum);
        ui->tableSum->style()->polish(ui->tableSum);
        
        ui->tableSum->clearContents();
        ui->tableSum->setRowCount(0);
        
        ui->tableSum->horizontalHeader()->style()->unpolish(ui->tableSum->horizontalHeader());
        ui->tableSum->horizontalHeader()->style()->polish(ui->tableSum->horizontalHeader());
        ui->tableSum->verticalHeader()->style()->unpolish(ui->tableSum->verticalHeader());
        ui->tableSum->verticalHeader()->style()->polish(ui->tableSum->verticalHeader());
    }
}

void Schedule_View::updateManagerPendingGrid(const QMap<int, QMap<int, ShiftBlock*>>& grid)
{
    ui->tableSum->clearContents();
    for (int col = 0; col < 7; ++col) {
        for (int row = 0; row < 14; ++row) {
            if (!grid.contains(col) || !grid[col].contains(row)) continue;
            
            int count = grid[col][row]->getStaffCount();
            
            QString text = QString("%1 yêu cầu").arg(count);
            QTableWidgetItem* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            if (count == 0) {
                item->setBackground(QColor(0xFE, 0xE2, 0xE2)); 
                item->setForeground(QColor(0x99, 0x1B, 0x1B));
            } else if (count < Config::getMinStaffPerShift()) {
                item->setBackground(QColor(0xFE, 0xF9, 0xC3)); 
                item->setForeground(QColor(0x85, 0x4D, 0x0E));
            } else {
                item->setBackground(QColor(0xD1, 0xFA, 0xE5)); 
                item->setForeground(QColor(0x06, 0x5F, 0x46));
            }
            ui->tableSum->setItem(row, col, item);
        }
    }
}

