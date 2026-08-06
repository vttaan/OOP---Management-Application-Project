#include "Salary_View.h"
#include "ui_Salary_View.h"
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDate>
#include <QBrush>

QString convertCurrency(QString salary) {
    bool ok = false;
    double val = salary.toDouble(&ok);
    if (ok) {
        salary = QString::number(static_cast<long long>(val));
    }
    QString validSalary;
    if (!salary.isEmpty() && salary[0] == '-') {
        validSalary += "-";
        salary = salary.sliced(1);
    }
    int index = ((salary.length() + 2) / 3) * 3 - salary.length();
    int i = 0;
    while (i < salary.length()) {
        validSalary += salary[i];
        if (index % 3 == 2 && i != salary.length() - 1) validSalary += ".";
        i++; index++;
    }
    validSalary += " VNĐ";
    return validSalary;
}

Salary_View::Salary_View(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Salary_View)
{
    ui->setupUi(this);
    
    QDate current = QDate::currentDate();
    
    for (int m = 1; m <= 12; ++m) {
        ui->monthComboBox->addItem(QString("Tháng %1").arg(m, 2, 10, QChar('0')), m);
    }
    
    int endYear = 2027;
    for (int y = 2020; y <= endYear; ++y) {
        ui->yearComboBox->addItem(QString::number(y), y);
    }
    
    ui->monthComboBox->blockSignals(true);
    ui->yearComboBox->blockSignals(true);
    
    int monthIndex = ui->monthComboBox->findData(current.month());
    if (monthIndex != -1) ui->monthComboBox->setCurrentIndex(monthIndex);
    
    int yearIndex = ui->yearComboBox->findData(current.year());
    if (yearIndex != -1) ui->yearComboBox->setCurrentIndex(yearIndex);
    
    ui->monthComboBox->blockSignals(false);
    ui->yearComboBox->blockSignals(false);

    setupUI();
    setupConnections();
}

Salary_View::~Salary_View()
{
    delete ui;
}

void Salary_View::setupUI()
{
    QString tableStyle = 
        "QTableWidget { "
        "   background-color: #FFFFFF; "
        "   gridline-color: #E2E8F0; "
        "   border: 1px solid #E2E8F0; "
        "   border-radius: 6px; "
        "   color: #334155; "
        "   outline: none; "
        "   font-size: 14px; "
        "} "
        "QTableWidget::item { "
        "   padding: 6px; "
        "} "
        "QHeaderView::section { "
        "   background-color: #F0F9FF; "
        "   color: #0284C7; "
        "   border: none; "
        "   border-right: 1px solid #E2E8F0; "
        "   border-bottom: 1px solid #E2E8F0; "
        "   font-weight: bold; "
        "   font-size: 14px; "
        "   padding: 8px; "
        "} "
        "QTableWidget::item:selected { "
        "   background-color: #E0F2FE; "
        "   color: #0369A1; "
        "} ";

    ui->normalTable->setRowCount(2);
    ui->normalTable->setShowGrid(true);
    ui->normalTable->setStyleSheet(tableStyle);
    QTableWidgetItem* normalDateHeader = new QTableWidgetItem("Ngày");
    ui->normalTable->setVerticalHeaderItem(0, normalDateHeader);
    QTableWidgetItem* normalHourHeader = new QTableWidgetItem("Số giờ");
    ui->normalTable->setVerticalHeaderItem(1, normalHourHeader);
    ui->normalTable->horizontalHeader()->setVisible(false);
    ui->normalTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->normalTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->normalTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    ui->holidayTable->setRowCount(2);
    ui->holidayTable->setShowGrid(true);
    ui->holidayTable->setStyleSheet(tableStyle);
    QTableWidgetItem* holidayDateHeader = new QTableWidgetItem("Ngày");
    ui->holidayTable->setVerticalHeaderItem(0, holidayDateHeader);
    QTableWidgetItem* holidayHourHeader = new QTableWidgetItem("Số giờ");
    ui->holidayTable->setVerticalHeaderItem(1, holidayHourHeader);
    ui->holidayTable->horizontalHeader()->setVisible(false);
    ui->holidayTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->holidayTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->holidayTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    ui->summaryTable->setRowCount(4);
    ui->summaryTable->setColumnCount(2);
    ui->summaryTable->setShowGrid(true);
    ui->summaryTable->setStyleSheet(tableStyle);
    ui->summaryTable->setHorizontalHeaderLabels({"Ngày thường", "Ngày lễ"});
    ui->summaryTable->setVerticalHeaderLabels({"Tổng giờ công", "Tổng lương", "Khoản phạt", "TỔNG"});
    ui->summaryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->summaryTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->summaryTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->summaryTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    ui->summaryTable->setSpan(2, 0, 1, 2);
    ui->summaryTable->setSpan(3, 0, 1, 2);
    
    ui->summaryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->normalTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->holidayTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    ui->normalTable->setTextElideMode(Qt::ElideLeft);
    ui->holidayTable->setTextElideMode(Qt::ElideLeft);
}

void Salary_View::setupConnections()
{
    auto onSelectionChanged = [=](){
        int month = ui->monthComboBox->currentData().toInt();
        int year = ui->yearComboBox->currentData().toInt();
        emit monthYearChanged(month, year);
    };
    connect(ui->monthComboBox, &QComboBox::currentTextChanged, this, [=](const QString&){ onSelectionChanged(); });
    connect(ui->yearComboBox, &QComboBox::currentTextChanged, this, [=](const QString&){ onSelectionChanged(); });

    connect(ui->normalTable, &QTableWidget::cellClicked, this, [=](int row, int col){
        QTableWidgetItem* dateItem = ui->normalTable->item(0, col);
        QTableWidgetItem* hoursItem = ui->normalTable->item(1, col);
        if (dateItem && hoursItem) {
            showDetailDialog(dateItem->text(), hoursItem->text(), "Ngày thường");
        }
    });

    connect(ui->holidayTable, &QTableWidget::cellClicked, this, [=](int row, int col){
        QTableWidgetItem* dateItem = ui->holidayTable->item(0, col);
        QTableWidgetItem* hoursItem = ui->holidayTable->item(1, col);
        if (dateItem && hoursItem) {
            showDetailDialog(dateItem->text(), hoursItem->text(), "Ngày lễ");
        }
    });
}

void Salary_View::setBaseSalary(const QString& salary)
{
    QString validSalary = "LCB: " + convertCurrency(salary);
    ui->lcbLabel->setText(validSalary);
}

void Salary_View::setEmployeeType(bool isFixed)
{
    this->isFixedEmployee = isFixed;
    if (isFixed) {
        ui->normalTable->setVisible(true);
        ui->holidayTable->setVisible(true);
        ui->normalTable->setRowHidden(1, true); // Hide hours row
        ui->holidayTable->setRowHidden(1, true); // Hide hours row
        
        ui->normalLabel->setVisible(true);
        ui->holidayLabel->setVisible(true);
        
        ui->summaryTable->setRowHidden(1, true); // Hide normal/holiday salary row
        ui->summaryTable->setRowHidden(2, false); // Show penalty row
        ui->summaryTable->setVerticalHeaderLabels({"Tổng ngày công", "Tổng lương", "Thưởng/phạt", "TỔNG"});
    } else {
        ui->normalTable->setVisible(true);
        ui->holidayTable->setVisible(true);
        ui->normalTable->setRowHidden(1, false);
        ui->holidayTable->setRowHidden(1, false);
        
        ui->normalLabel->setVisible(true);
        ui->holidayLabel->setVisible(true);
        
        ui->summaryTable->setRowHidden(1, false); // Show normal/holiday salary row
        ui->summaryTable->setRowHidden(2, true); // Hide penalty row
        ui->summaryTable->setVerticalHeaderLabels({"Tổng giờ công", "Tổng lương", "Khoản phạt", "TỔNG"});
    }
}

void Salary_View::populateNormalTable(const QMap<QString, int>& data)
{
    ui->normalTable->setColumnCount(data.size());
    ui->normalTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->normalTable->horizontalHeader()->setMinimumSectionSize(60);
    int col = 0;
    for(auto it = data.begin(); it != data.end(); ++it) {
        QTableWidgetItem* dateItem = new QTableWidgetItem(it.key());
        dateItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        dateItem->setForeground(QBrush(QColor("#0369A1")));
        QFont f = dateItem->font();
        f.setBold(true);
        dateItem->setFont(f);
        ui->normalTable->setItem(0, col, dateItem);

        QTableWidgetItem* hoursItem = new QTableWidgetItem(QString::number(it.value()));
        hoursItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        hoursItem->setForeground(QBrush(QColor("#334155")));
        ui->normalTable->setItem(1, col, hoursItem);
        col++;
    }
}

void Salary_View::populateHolidayTable(const QMap<QString, int>& data)
{
    ui->holidayTable->setColumnCount(data.size());
    ui->holidayTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->holidayTable->horizontalHeader()->setMinimumSectionSize(60);
    int col = 0;
    for(auto it = data.begin(); it != data.end(); ++it) {
        QTableWidgetItem* dateItem = new QTableWidgetItem(it.key());
        dateItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        dateItem->setForeground(QBrush(QColor("#0369A1")));
        QFont f = dateItem->font();
        f.setBold(true);
        dateItem->setFont(f);
        ui->holidayTable->setItem(0, col, dateItem);

        QTableWidgetItem* hoursItem = new QTableWidgetItem(QString::number(it.value()));
        hoursItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        hoursItem->setForeground(QBrush(QColor("#334155")));
        ui->holidayTable->setItem(1, col, hoursItem);
        col++;
    }
}

void Salary_View::populateSummaryTable(const SalaryData& data)
{
    QString normalHoursText = QString::number(data.normalHours) + (isFixedEmployee ? " ngày" : " giờ");
    QString holidayHoursText = QString::number(data.holidayHours) + (isFixedEmployee ? " ngày" : " giờ");

    QTableWidgetItem* normalItem = new QTableWidgetItem(normalHoursText);
    normalItem->setForeground(QBrush(QColor("#334155")));
    ui->summaryTable->setItem(0, 0, normalItem);
    
    QTableWidgetItem* holidayItem = new QTableWidgetItem(holidayHoursText);
    holidayItem->setForeground(QBrush(QColor("#334155")));
    ui->summaryTable->setItem(0, 1, holidayItem);
    
    QTableWidgetItem* normalSalaryItem = new QTableWidgetItem(convertCurrency(QString::number(data.normalSalary)));
    normalSalaryItem->setForeground(QBrush(QColor("#0369A1")));
    QFont fn = normalSalaryItem->font(); fn.setBold(true); normalSalaryItem->setFont(fn);
    ui->summaryTable->setItem(1, 0, normalSalaryItem);
    
    QTableWidgetItem* holidaySalaryItem = new QTableWidgetItem(convertCurrency(QString::number(data.holidaySalary)));
    holidaySalaryItem->setForeground(QBrush(QColor("#0369A1")));
    QFont fh = holidaySalaryItem->font(); fh.setBold(true); holidaySalaryItem->setFont(fh);
    ui->summaryTable->setItem(1, 1, holidaySalaryItem);
    
    QString penaltyText = convertCurrency(QString::number(data.penalty));
                          
    QTableWidgetItem* penaltyItem = new QTableWidgetItem(penaltyText);
    penaltyItem->setTextAlignment(Qt::AlignCenter);
    if (data.penalty > 0) {
        penaltyItem->setForeground(QBrush(QColor("#16A34A"))); // Green 600
        penaltyItem->setText("+" + penaltyText);
    } else {
        penaltyItem->setForeground(QBrush(QColor("#E11D48"))); // Rose 600
    }
    ui->summaryTable->setItem(2, 0, penaltyItem);
    
    QTableWidgetItem* totalItem = new QTableWidgetItem(convertCurrency(QString::number(data.totalSalary)));
    totalItem->setTextAlignment(Qt::AlignCenter);
    totalItem->setForeground(QBrush(QColor("#0284C7"))); // Sky 600
    QFont ft = totalItem->font(); ft.setBold(true); ft.setPointSize(11); totalItem->setFont(ft);
    ui->summaryTable->setItem(3, 0, totalItem);
}

void Salary_View::showDetailDialog(const QString& date, const QString& hours, const QString& type)
{
    QDialog dialog(this);
    dialog.setWindowTitle("Chi tiết ngày công");
    dialog.setMinimumWidth(320);
    dialog.setStyleSheet(
        "QDialog {"
        "    background-color: #FFFFFF;"
        "    border-radius: 12px;"
        "}"
        "QLabel#titleLabel {"
        "    color: #0F172A;"
        "    font-weight: bold;"
        "    font-size: 16px;"
        "}"
        "QLabel#infoLabel {"
        "    color: #475569;"
        "    font-size: 14px;"
        "}"
        "QPushButton {"
        "    background-color: #0284C7;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 8px 16px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #0369A1;"
        "}"
    );

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    QLabel* titleLabel = new QLabel("Chi tiết ngày công", &dialog);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    QFrame* contentFrame = new QFrame(&dialog);
    contentFrame->setStyleSheet(
        "QFrame {"
        "    background-color: #F8FAFC;"
        "    border: 1px solid #E2E8F0;"
        "    border-radius: 8px;"
        "    padding: 12px;"
        "}"
    );
    QVBoxLayout* contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setSpacing(8);

    QLabel* typeLabel = new QLabel(QString("<b>Loại ngày:</b> %1").arg(type), &dialog);
    typeLabel->setObjectName("infoLabel");
    contentLayout->addWidget(typeLabel);

    QLabel* dateLabel = new QLabel(QString("<b>Ngày:</b> %1").arg(date), &dialog);
    dateLabel->setObjectName("infoLabel");
    contentLayout->addWidget(dateLabel);

    QString unit = (isFixedEmployee) ? "ngày" : "giờ";
    QLabel* hoursLabel = new QLabel(QString("<b>Giờ công:</b> %1 %2").arg(hours, unit), &dialog);
    hoursLabel->setObjectName("infoLabel");
    contentLayout->addWidget(hoursLabel);
    
    if (isFixedEmployee) {
        hoursLabel->setVisible(false);
    }

    layout->addWidget(contentFrame);

    QPushButton* closeBtn = new QPushButton("Đóng", &dialog);
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignCenter);

    dialog.exec();
}


