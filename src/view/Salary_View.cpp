#include "Salary_View.h"
#include "ui_Salary_View.h"
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDate>

QString convertCurrency(QString salary) {
    QString validSalary;
    if (salary[0] == '-') {
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
    ui->normalTable->setRowCount(2);
    ui->normalTable->setVerticalHeaderItem(0, new QTableWidgetItem("Ngày"));
    ui->normalTable->setVerticalHeaderItem(1, new QTableWidgetItem("Số giờ"));
    ui->normalTable->horizontalHeader()->setVisible(false);
    ui->normalTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->normalTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->normalTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    ui->holidayTable->setRowCount(2);
    ui->holidayTable->setVerticalHeaderItem(0, new QTableWidgetItem("Ngày"));
    ui->holidayTable->setVerticalHeaderItem(1, new QTableWidgetItem("Số giờ"));
    ui->holidayTable->horizontalHeader()->setVisible(false);
    ui->holidayTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->holidayTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->holidayTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    ui->summaryTable->setRowCount(4);
    ui->summaryTable->setColumnCount(2);
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
}

void Salary_View::setBaseSalary(const QString& salary)
{
    QString validSalary = "LCB: " + convertCurrency(salary);
    ui->lcbLabel->setText(validSalary);
}

void Salary_View::setRole(const QString& role)
{
    this->currentRole = role;
    if (role == "Manager") {
        ui->normalTable->setVisible(false);
        ui->holidayTable->setVisible(false);
        ui->normalLabel->setVisible(false);
        ui->holidayLabel->setVisible(false);
        ui->summaryTable->setVerticalHeaderLabels({"Tổng ngày công", "Tổng lương", "Khoản phạt", "TỔNG"});
    } else {
        ui->normalTable->setVisible(true);
        ui->holidayTable->setVisible(true);
        ui->normalLabel->setVisible(true);
        ui->holidayLabel->setVisible(true);
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
        dateItem->setTextAlignment(Qt::AlignCenter);
        ui->normalTable->setItem(0, col, dateItem);

        QTableWidgetItem* hoursItem = new QTableWidgetItem(QString::number(it.value()));
        hoursItem->setTextAlignment(Qt::AlignCenter);
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
        dateItem->setTextAlignment(Qt::AlignCenter);
        ui->holidayTable->setItem(0, col, dateItem);

        QTableWidgetItem* hoursItem = new QTableWidgetItem(QString::number(it.value()));
        hoursItem->setTextAlignment(Qt::AlignCenter);
        ui->holidayTable->setItem(1, col, hoursItem);
        col++;
    }
}

void Salary_View::populateSummaryTable(const SalaryData& data)
{
    QString normalHoursText = QString::number(data.normalHours) + (currentRole == "Manager" ? " ngày" : " giờ");
    QString holidayHoursText = QString::number(data.holidayHours) + (currentRole == "Manager" ? " ngày" : " giờ");

    ui->summaryTable->setItem(0, 0, new QTableWidgetItem(normalHoursText));
    ui->summaryTable->setItem(0, 1, new QTableWidgetItem(holidayHoursText));
    
    ui->summaryTable->setItem(1, 0, new QTableWidgetItem(convertCurrency(QString::number(data.normalSalary))));
    ui->summaryTable->setItem(1, 1, new QTableWidgetItem(convertCurrency(QString::number(data.holidaySalary))));
    
    QString penaltyText = convertCurrency(QString::number(data.penalty));
                          
    QTableWidgetItem* penaltyItem = new QTableWidgetItem(penaltyText);
    penaltyItem->setTextAlignment(Qt::AlignCenter);
    ui->summaryTable->setItem(2, 0, penaltyItem);
    
    QTableWidgetItem* totalItem = new QTableWidgetItem(convertCurrency(QString::number(data.totalSalary)));
    totalItem->setTextAlignment(Qt::AlignCenter);
    ui->summaryTable->setItem(3, 0, totalItem);
}


