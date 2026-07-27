#include "ViewSchedule_View.h"
#include "ui_ViewSchedule_View.h"
#include "utils/Config.h"
#include "core/ShiftBlock.h"
#include <QMessageBox>
#include <QSplitter>

ViewSchedule_View::ViewSchedule_View(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ViewSchedule_View)
{
    ui->setupUi(this);
    setUpUI();
    connect(ui->btnPrevWeek, &QPushButton::clicked, this, &ViewSchedule_View::onBtnPrevClicked);
    connect(ui->btnNextWeek, &QPushButton::clicked, this, &ViewSchedule_View::onBtnNextClicked);
    connect(ui->btnCurrentWeek, &QPushButton::clicked, this, &ViewSchedule_View::onBtnCurrentClicked);
    connect(ui->tableSchedule, &QTableWidget::cellClicked, this, [this](int row, int col) {
        emit shiftClicked(row, col);
    });
}

ViewSchedule_View::~ViewSchedule_View()
{
    delete ui;
}

void ViewSchedule_View::setUpUI()
{
    this->setStyleSheet("background-color: #FFFFFF;");

    ui->btnPrevWeek->setStyleSheet(ScheduleStyle::BtnNormal);
    ui->btnNextWeek->setStyleSheet(ScheduleStyle::BtnNormal);
    ui->btnCurrentWeek->setStyleSheet(ScheduleStyle::BtnHighlight);
    
    // Table styles
    ui->tableSchedule->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableSchedule->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableSchedule->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableSchedule->setFocusPolicy(Qt::NoFocus);
    
    // Force uppercase for horizontal headers
    for(int i = 0; i < ui->tableSchedule->columnCount(); ++i) {
        if(ui->tableSchedule->horizontalHeaderItem(i)) {
            ui->tableSchedule->horizontalHeaderItem(i)->setText(ui->tableSchedule->horizontalHeaderItem(i)->text().toUpper());
        }
    }
    
    ui->tableSchedule->setProperty("role", "staff");

    // Create Splitter and Bottom Layout
    ui->verticalLayout->removeWidget(ui->tableSchedule);
    
    splitter = new QSplitter(Qt::Vertical, this);
    ui->verticalLayout->addWidget(splitter);
    splitter->addWidget(ui->tableSchedule);
    
    bottomWidget = new QWidget(this);
    QHBoxLayout* bottomLayout = new QHBoxLayout(bottomWidget);
    bottomLayout->setContentsMargins(0, 10, 0, 0);
    
    QVBoxLayout* leftLayout = new QVBoxLayout();
    QLabel* lblMissing = new QLabel("CA LÀM THIẾU NHÂN VIÊN", bottomWidget);
    lblMissing->setStyleSheet(ScheduleStyle::Title);
    
    tableMissingStaff = new QTableWidget(this);
    tableMissingStaff->setColumnCount(2);
    tableMissingStaff->setHorizontalHeaderLabels({"CA LÀM VIỆC THIẾU NGƯỜI", "SL HIỆN CÓ / YÊU CẦU"});
    tableMissingStaff->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableMissingStaff->setProperty("role", "manager-dark");
    tableMissingStaff->setSelectionMode(QAbstractItemView::NoSelection);
    tableMissingStaff->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableMissingStaff->setFocusPolicy(Qt::NoFocus);
    
    leftLayout->addWidget(lblMissing);
    leftLayout->addWidget(tableMissingStaff);
    
    QVBoxLayout* rightLayout = new QVBoxLayout();
    QLabel* lblDetails = new QLabel("THÔNG TIN NHÂN VIÊN TRONG CA LÀM", bottomWidget);
    lblDetails->setStyleSheet(ScheduleStyle::Title);

    tableShiftDetails = new QTableWidget(this);
    tableShiftDetails->setColumnCount(5);
    tableShiftDetails->setHorizontalHeaderLabels({"STT", "ID", "TÊN", "VAI TRÒ", "SĐT"});
    tableShiftDetails->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableShiftDetails->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tableShiftDetails->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    tableShiftDetails->setProperty("role", "manager-dark");
    tableShiftDetails->setSelectionMode(QAbstractItemView::NoSelection);
    tableShiftDetails->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableShiftDetails->setFocusPolicy(Qt::NoFocus);
    
    rightLayout->addWidget(lblDetails);
    rightLayout->addWidget(tableShiftDetails);
    
    bottomLayout->addLayout(leftLayout, 4);
    bottomLayout->addLayout(rightLayout, 6);
    
    splitter->addWidget(bottomWidget);
    splitter->setSizes({600, 400});
}

void ViewSchedule_View::updateTable(const QMap<int, QList<QString>>& weeklyData)
{
    ui->tableSchedule->verticalHeader()->setVisible(false);
    int maxRow = 1;
    for(auto it = weeklyData.begin(); it != weeklyData.end(); it++) {
        maxRow = qMax(maxRow, (int)it.value().size());
    }
    ui->tableSchedule->setRowCount(maxRow);
    ui->tableSchedule->clearContents();
    
    for(auto it = weeklyData.begin(); it != weeklyData.end(); it++) {
        QList<QString> shift = it.value();
        for(int j = 0; j < shift.size(); j++) {
            QTableWidgetItem* item = new QTableWidgetItem(shift[j]);
            item->setTextAlignment(Qt::AlignCenter);
            item->setForeground(QBrush(Qt::black));

            item->setBackground(QColor(0xF3, 0xF4, 0xF6));
            item->setForeground(QColor(0x37, 0x41, 0x51));
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
            ui->tableSchedule->setItem(j, it.key(), item);
        }
    }
}

void ViewSchedule_View::updateManagerTable(const QList<QString>& timeSlots, const QMap<int, QMap<int, ShiftBlock*>>& gridData)
{
    ui->tableSchedule->setRowCount(timeSlots.size());
    ui->tableSchedule->setColumnCount(7);

    // Set vertical headers
    ui->tableSchedule->setVerticalHeaderLabels(timeSlots);
    ui->tableSchedule->verticalHeader()->setVisible(true);

    // Style vertical headers similar to horizontal (Pastel Purple for Manager)
    // (Handled by QSS property now)

    // Populate data
    for (int col = 0; col < 7; ++col) {
        if (!gridData.contains(col)) continue;
        const QMap<int, ShiftBlock*>& dayData = gridData[col];
        
        for (auto it = dayData.begin(); it != dayData.end(); ++it) {
            int row = it.key();
            ShiftBlock* block = it.value();
            if (!block) continue;
            
            QTableWidgetItem* item = new QTableWidgetItem(block->getDisplayText());
            item->setTextAlignment(Qt::AlignCenter);

            if (block->getStatus() == ShiftStatus::Empty) {
                item->setText("Không đạt");
                item->setBackground(QColor(0xFE, 0xE2, 0xE2)); // pastel red
                item->setForeground(QColor(0x99, 0x1B, 0x1B)); // dark red
            } else if (block->getStatus() == ShiftStatus::Understaffed) {
                item->setBackground(QColor(0xFE, 0xF9, 0xC3)); // pastel yellow
                item->setForeground(QColor(0x85, 0x4D, 0x0E)); // dark yellow
            } else {
                item->setBackground(QColor(0xD1, 0xFA, 0xE5)); // emerald-100
                item->setForeground(QColor(0x06, 0x5F, 0x46)); // emerald-800
            }
            
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
            ui->tableSchedule->setItem(row, col, item);
        }
    }
}

void ViewSchedule_View::updateDateRange(const QString& dateRangeText)
{
    ui->lblDateRange->setText(dateRangeText);
}

void ViewSchedule_View::highlightToday(int currentDayIndex)
{
    bool isManager = (ui->tableSchedule->property("role").toString() == "manager");
    QColor defaultTextColor = isManager ? Qt::white : Qt::black;

    // Reset all headers to default first
    for(int i = 0; i < 7; ++i) {
        QTableWidgetItem* headerItem = ui->tableSchedule->horizontalHeaderItem(i);
        if(!headerItem) {
            headerItem = new QTableWidgetItem();
            ui->tableSchedule->setHorizontalHeaderItem(i, headerItem);
        }
        headerItem->setBackground(QBrush());
        headerItem->setForeground(defaultTextColor);
        QFont f = headerItem->font();
        f.setBold(true);
        headerItem->setFont(f);
    }

    if(currentDayIndex >= 0 && currentDayIndex <= 6) {
        QTableWidgetItem* todayItem = ui->tableSchedule->horizontalHeaderItem(currentDayIndex);
        if(todayItem) {

            todayItem->setBackground(QColor(0xBA, 0xE6, 0xFD));
            todayItem->setForeground(QColor(Qt::black));
            QFont f = todayItem->font();
            f.setBold(true);
            todayItem->setFont(f);
        }
        
        for (int row = 0; row < ui->tableSchedule->rowCount(); ++row) {
            QTableWidgetItem* cellItem = ui->tableSchedule->item(row, currentDayIndex);
            if (cellItem) {
                cellItem->setBackground(QColor(0xE0, 0xF2, 0xFE)); // Light blue
                cellItem->setForeground(QColor(0x03, 0x69, 0xA1)); // Blue text
            }
        }
    }
}

void ViewSchedule_View::updateUnqualifiedShifts(const QList<QPair<QString, int>>& unqualifiedShifts)
{
    tableMissingStaff->setRowCount(unqualifiedShifts.size());
    tableMissingStaff->clearContents();
    
    for (int i = 0; i < unqualifiedShifts.size(); ++i) {
        QString shiftName = unqualifiedShifts[i].first;
        int count = unqualifiedShifts[i].second;

        QTableWidgetItem* itemCa = new QTableWidgetItem(shiftName);
        itemCa->setTextAlignment(Qt::AlignCenter);
        QFont f1 = itemCa->font(); f1.setBold(true); itemCa->setFont(f1);
        
        QTableWidgetItem* itemCount = new QTableWidgetItem(QString::number(count) + " / " + QString::number(Config::getMinStaffPerShift()));
        itemCount->setTextAlignment(Qt::AlignCenter);
        QFont f = itemCount->font(); f.setBold(true); itemCount->setFont(f);
        
        if (count == 0) { // Empty -> Pastel Red
            itemCa->setBackground(QColor(0xFE, 0xE2, 0xE2)); 
            itemCa->setForeground(QColor(0x99, 0x1B, 0x1B)); 
            itemCount->setBackground(QColor(0xFE, 0xE2, 0xE2));
            itemCount->setForeground(QColor(0x99, 0x1B, 0x1B));
        } else { // Understaffed -> Pastel Yellow
            itemCa->setBackground(QColor(0xFE, 0xF9, 0xC3)); 
            itemCa->setForeground(QColor(0x85, 0x4D, 0x0E)); 
            itemCount->setBackground(QColor(0xFE, 0xF9, 0xC3)); 
            itemCount->setForeground(QColor(0x85, 0x4D, 0x0E));
        }

        tableMissingStaff->setItem(i, 0, itemCa);
        tableMissingStaff->setItem(i, 1, itemCount);
    }
}

void ViewSchedule_View::updateShiftDetails(const QList<User*>& employees, const QString& timeLabel)
{
    tableShiftDetails->setRowCount(employees.size());
    tableShiftDetails->clearContents();
    
    for (int i = 0; i < employees.size(); ++i) {
        User* emp = employees[i];
        
        QTableWidgetItem* itemSTT = new QTableWidgetItem(QString::number(i + 1));
        itemSTT->setTextAlignment(Qt::AlignCenter);
        itemSTT->setForeground(QBrush(Qt::black));
        
        QTableWidgetItem* itemID = new QTableWidgetItem(QString::number(emp->getIdEmployee()));
        itemID->setTextAlignment(Qt::AlignCenter);
        itemID->setForeground(QBrush(Qt::black));
        
        QTableWidgetItem* itemName = new QTableWidgetItem(emp->getName());
        itemName->setTextAlignment(Qt::AlignCenter);
        itemName->setForeground(QBrush(Qt::black));
        
        QTableWidgetItem* itemRole = new QTableWidgetItem((emp->getRole() == "Staff" ? "Nhân viên" : "Quản lý"));
        itemRole->setTextAlignment(Qt::AlignCenter);
        itemRole->setForeground(QBrush(Qt::black));
        
        QTableWidgetItem* itemPhone = new QTableWidgetItem(emp->getPhoneNum());
        itemPhone->setTextAlignment(Qt::AlignCenter);
        itemPhone->setForeground(QBrush(Qt::black));

        QFont f = itemSTT->font(); f.setBold(true);
        itemSTT->setFont(f);
        itemID->setFont(f);
        itemName->setFont(f);
        itemRole->setFont(f);
        itemPhone->setFont(f);

        tableShiftDetails->setItem(i, 0, itemSTT);
        tableShiftDetails->setItem(i, 1, itemID);
        tableShiftDetails->setItem(i, 2, itemName);
        tableShiftDetails->setItem(i, 3, itemRole);
        tableShiftDetails->setItem(i, 4, itemPhone);
    }
}

void ViewSchedule_View::onBtnPrevClicked() { emit requestPrevWeek(); }
void ViewSchedule_View::onBtnNextClicked() { emit requestNextWeek(); }
void ViewSchedule_View::onBtnCurrentClicked() { emit requestCurrentWeek(); }

void ViewSchedule_View::setManagerFeaturesVisible(bool visible) {
    if (bottomWidget) {
        bottomWidget->setVisible(visible);
    }


    ui->tableSchedule->setProperty("role", visible ? "manager" : "staff");
    ui->tableSchedule->style()->unpolish(ui->tableSchedule);
    ui->tableSchedule->style()->polish(ui->tableSchedule);
    
    ui->tableSchedule->horizontalHeader()->style()->unpolish(ui->tableSchedule->horizontalHeader());
    ui->tableSchedule->horizontalHeader()->style()->polish(ui->tableSchedule->horizontalHeader());
    
    if (ui->tableSchedule->verticalHeader()) {
        ui->tableSchedule->verticalHeader()->style()->unpolish(ui->tableSchedule->verticalHeader());
        ui->tableSchedule->verticalHeader()->style()->polish(ui->tableSchedule->verticalHeader());
    }
}
