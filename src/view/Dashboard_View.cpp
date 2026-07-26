#include "Dashboard_View.h"
#include "ui_Dashboard_View.h"
#include "control/Dashboard_Control.h"
#include "employeecard.h"
Dashboard_View::Dashboard_View(Dashboard_Control *controller, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Dashboard_View)
    , controller(controller)
{
    ui->setupUi(this);
    connect(ui->txtSearch, &QLineEdit::textChanged,
            this, &Dashboard_View::searchChanged);
}
Dashboard_View::~Dashboard_View() { delete ui; }
void Dashboard_View::updateStatCards(int total, int staff, int manager, int workingToday)
{
    ui->lblNum1->setText(QString::number(total));
    ui->lblNum2->setText(QString::number(staff));
    ui->lblNum3->setText(QString::number(manager));
    ui->lblNum4->setText(QString::number(workingToday));
    ui->lblSub4->setText(total > 0
                             ? QString("%1% nhân sự đi làm").arg(workingToday * 100 / total)
                             : "0% nhân sự đi làm");
}
void Dashboard_View::clearEmployeeGrid()
{
    QLayout* old = ui->scrollAreaWidgetContents->layout();
    if (!old) return;
    QLayoutItem* item;
    while ((item = old->takeAt(0))) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    delete old;
}
void Dashboard_View::addEmployeeCard(EmployeeCard* card)
{
    QGridLayout* grid = qobject_cast<QGridLayout*>(
        ui->scrollAreaWidgetContents->layout());
    if (!grid) {
        grid = new QGridLayout(ui->scrollAreaWidgetContents);
        grid->setSpacing(16);
        grid->setContentsMargins(16, 16, 16, 16);
    }
    const int COLS = 5;
    int count = grid->count();
    int row   = count / COLS;
    int col   = count % COLS;
    card->setParent(ui->scrollAreaWidgetContents);
    grid->addWidget(card, row, col);
}
void Dashboard_View::updateShiftPanel(
    const QList<QPair<QString, QString>>& nextShifts,
    const QStringList& absentNames)
{
    while (QLayoutItem* i = ui->layoutNextShift->takeAt(0)) {
        if (i->widget()) i->widget()->deleteLater();
        delete i;
    }
    if (nextShifts.isEmpty()) {
        auto* lbl = new QLabel("Không có ca nào sắp tới");
        lbl->setStyleSheet("color: #919eab; font-style: italic; padding: 4px 0;");
        ui->layoutNextShift->addWidget(lbl);
    }
    for (const auto& s : nextShifts) {
        auto* row = new QLabel(QString("⏰  %1   (%2)").arg(s.first).arg(s.second));
        row->setStyleSheet(
            "color: #212b36; padding: 6px 8px; border-bottom: 1px solid #f0f0f0;");
        ui->layoutNextShift->addWidget(row);
    }
    while (QLayoutItem* i = ui->layoutAbsent->takeAt(0)) {
        if (i->widget()) i->widget()->deleteLater();
        delete i;
    }
    if (absentNames.isEmpty()) {
        auto* lbl = new QLabel("Tất cả nhân viên đã đi làm 🎉");
        lbl->setStyleSheet("color: #1e8e3e; font-style: italic; padding: 4px 0;");
        ui->layoutAbsent->addWidget(lbl);
    }
    for (const auto& name : absentNames) {
        auto* row = new QLabel("🔴  " + name);
        row->setStyleSheet(
            "color: #c5221f; padding: 6px 8px; border-bottom: 1px solid #f0f0f0;");
        ui->layoutAbsent->addWidget(row);
    }
}