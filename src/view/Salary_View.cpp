#include "Salary_View.h"
#include "ui_Salary_View.h"
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDate>

Salary_View::Salary_View(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Salary_View)
{
    ui->setupUi(this);
    setupUI();
    setupConnections();
    
    QDate current = QDate::currentDate();
    ui->monthYearComboBox->setCurrentText(QString("%1/%2").arg(current.month(), 2, 10, QChar('0')).arg(current.year()));
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
    
    ui->holidayTable->setRowCount(2);
    ui->holidayTable->setVerticalHeaderItem(0, new QTableWidgetItem("Ngày"));
    ui->holidayTable->setVerticalHeaderItem(1, new QTableWidgetItem("Số giờ"));
    ui->holidayTable->horizontalHeader()->setVisible(false);

    ui->summaryTable->setRowCount(4);
    ui->summaryTable->setColumnCount(2);
    ui->summaryTable->setHorizontalHeaderLabels({"Ngày thường", "Ngày lễ"});
    ui->summaryTable->setVerticalHeaderLabels({"Tổng giờ công", "Tổng lương", "Khoản phạt", "TỔNG"});
    ui->summaryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    ui->summaryTable->setSpan(2, 0, 1, 2);
    ui->summaryTable->setSpan(3, 0, 1, 2);
    
    ui->summaryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->normalTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->holidayTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void Salary_View::setupConnections()
{
    connect(ui->monthYearComboBox, &QComboBox::currentTextChanged, this, [=](const QString& text){
        QStringList parts = text.split("/");
        if(parts.size() == 2) {
            emit monthYearChanged(parts[0].toInt(), parts[1].toInt());
        }
    });
}

void Salary_View::setBaseSalary(const QString& salary)
{
    ui->lcbLabel->setText("LCB: " + salary);
}

void Salary_View::populateNormalTable(const QMap<QString, int>& data)
{
    ui->normalTable->setColumnCount(data.size());
    int col = 0;
    for(auto it = data.begin(); it != data.end(); ++it) {
        ui->normalTable->setItem(0, col, new QTableWidgetItem(it.key()));
        ui->normalTable->setItem(1, col, new QTableWidgetItem(QString::number(it.value())));
        col++;
    }
}

void Salary_View::populateHolidayTable(const QMap<QString, int>& data)
{
    ui->holidayTable->setColumnCount(data.size());
    int col = 0;
    for(auto it = data.begin(); it != data.end(); ++it) {
        ui->holidayTable->setItem(0, col, new QTableWidgetItem(it.key()));
        ui->holidayTable->setItem(1, col, new QTableWidgetItem(QString::number(it.value())));
        col++;
    }
}

void Salary_View::pobackground-color: #f8f9fa;pulateSummaryTable(const SalaryData& data)
{
    ui->summaryTable->setItem(0, 0, new QTableWidgetItem(QString::number(data.normalHours)));
    ui->summaryTable->setItem(0, 1, new QTableWidgetItem(QString::number(data.holidayHours)));
    
    ui->summaryTable->setItem(1, 0, new QTableWidgetItem(data.normalSalary));
    ui->summaryTable->setItem(1, 1, new QTableWidgetItem(data.holidaySalary));
    
    QTableWidgetItem* penaltyItem = new QTableWidgetItem(data.penalty);
    penaltyItem->setTextAlignment(Qt::AlignCenter);
    ui->summaryTable->setItem(2, 0, penaltyItem);
    
    QTableWidgetItem* totalItem = new QTableWidgetItem(data.totalSalary);
    totalItem->setTextAlignment(Qt::AlignCenter);
    ui->summaryTable->setItem(3, 0, totalItem);
}
