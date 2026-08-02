#include "ViewSchedule_View.h"
#include "ui_ViewSchedule_View.h"
#include "core/ShiftBlock.h"
#include "core/User.h"
#include "core/Shift.h"

ViewSchedule_View::ViewSchedule_View(QWidget *parent) : QWidget(parent),
                                                        ui(new Ui::ViewSchedule_View)
{
    ui->setupUi(this);
    setUpUI();
    connect(ui->btnPrevWeek, &QPushButton::clicked, this, &ViewSchedule_View::onBtnPrevClicked);
    connect(ui->btnNextWeek, &QPushButton::clicked, this, &ViewSchedule_View::onBtnNextClicked);
    connect(ui->btnCurrentWeek, &QPushButton::clicked, this, &ViewSchedule_View::onBtnCurrentClicked);
    connect(ui->tableSchedule, &QTableWidget::cellClicked, this, [this](int row, int col)
            { emit shiftClicked(row, col); });
}

ViewSchedule_View::~ViewSchedule_View()
{
    delete ui;
}

void ViewSchedule_View::setUpUI()
{
    this->setObjectName("ViewScheduleMain");
    this->setStyleSheet("#ViewScheduleMain { background-color: #F8FAFC; } QWidget { color: #1F2937; } QLabel { background-color: transparent; }");

    ui->btnPrevWeek->setStyleSheet(
        "QPushButton { background-color: transparent; color: #374151; border: none; "
        "border-radius: 4px; padding: 2px 6px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background-color: #E5E7EB; }");
    ui->btnNextWeek->setStyleSheet(
        "QPushButton { background-color: transparent; color: #374151; border: none; "
        "border-radius: 4px; padding: 2px 6px; font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background-color: #E5E7EB; }");
    ui->btnCurrentWeek->setStyleSheet(ScheduleStyle::BtnHighlight);

    // ── Pending table (staff mode — 1 row x 7 cols) ──
    ui->tablePending->setRowCount(1);
    ui->tablePending->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tablePending->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tablePending->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tablePending->setFocusPolicy(Qt::NoFocus);
    ui->tablePending->verticalHeader()->setVisible(false);
    ui->tablePending->setWordWrap(true);
    ui->tablePending->setStyleSheet(
        "QTableWidget { border: none; background-color: transparent; }"
        "QHeaderView::section { background-color: #D97706; color: white; font-weight: bold; "
        "  padding: 5px; border: none; }"
        "QTableWidget::item { padding: 6px; color: #92400E; background-color: #FFFBEB; border: 1px solid #FDE68A; }");
    // Force uppercase for pending table headers
    for (int i = 0; i < ui->tablePending->columnCount(); ++i)
    {
        if (ui->tablePending->horizontalHeaderItem(i))
            ui->tablePending->horizontalHeaderItem(i)->setText(
                ui->tablePending->horizontalHeaderItem(i)->text().toUpper());
    }

    ui->tableSchedule->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableSchedule->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tablePending->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tablePending->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Prevent the title labels from expanding and taking up empty space
    ui->lblPendingTitle->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    
    // Move pendingFrame under approvedFrame in the layout
    ui->scheduleLayout->removeWidget(ui->pendingFrame);
    ui->scheduleLayout->addWidget(ui->pendingFrame);
    ui->tableSchedule->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableSchedule->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableSchedule->setFocusPolicy(Qt::NoFocus);
    ui->tableSchedule->setMinimumHeight(150);
    ui->tablePending->setMinimumHeight(150);
    ui->tablePending->setMaximumHeight(QWIDGETSIZE_MAX);
    ui->tableSchedule->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->tablePending->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->tableSchedule->setWordWrap(true);

    // Start in staff mode: 1 row, no vertical header
    ui->tableSchedule->setRowCount(1);
    ui->tableSchedule->verticalHeader()->setVisible(false);

    // Horizontal header uppercase
    for (int i = 0; i < ui->tableSchedule->columnCount(); ++i)
    {
        if (ui->tableSchedule->horizontalHeaderItem(i))
        {
            ui->tableSchedule->horizontalHeaderItem(i)->setText(
                ui->tableSchedule->horizontalHeaderItem(i)->text().toUpper());
        }
    }
    ui->tableSchedule->setProperty("role", "staff");

    // ── Build bottom splitter + details pane ──
    ui->verticalLayout->addWidget(ui->scheduleCard, 7);

    detailsWidget = new QFrame(this);
    detailsWidget->setObjectName("detailsCard");
    detailsWidget->setStyleSheet(
        "QFrame#detailsCard { "
        "background-color: #FFFFFF; "
        "border: 1px solid #E5E7EB; "
        "border-radius: 14px; "
        "}");

    QVBoxLayout *detailsLayout = new QVBoxLayout(detailsWidget);
    detailsLayout->setContentsMargins(16, 16, 16, 16);

    lblShiftDetailTitle = new QLabel("THÔNG TIN NHÂN VIÊN TRONG CA LÀM", detailsWidget);
    lblShiftDetailTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #1F2937; padding: 6px 10px; background-color: #E5E7EB; border-radius: 6px;");

    tableShiftDetails = new QTableWidget(this);
    tableShiftDetails->setColumnCount(6);
    tableShiftDetails->setHorizontalHeaderLabels({"STT", "ID", "TÊN", "VAI TRÒ", "SĐT", "GIỜ LÀM"});
    tableShiftDetails->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableShiftDetails->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tableShiftDetails->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    tableShiftDetails->setSelectionMode(QAbstractItemView::NoSelection);
    tableShiftDetails->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableShiftDetails->setFocusPolicy(Qt::NoFocus);
    tableShiftDetails->setStyleSheet(
        "QTableWidget { background-color: transparent; border: none; }"
        "QHeaderView::section { background-color: #2F80ED; color: white; font-weight: bold; padding: 6px; border: none; }"
        "QTableWidget::item { padding: 6px; color: #1F2937; }"
        "QTableWidget::item:alternate { background-color: #EFF6FF; }");
    tableShiftDetails->setAlternatingRowColors(true);

    detailsLayout->addWidget(lblShiftDetailTitle);
    detailsLayout->addWidget(tableShiftDetails);

    ui->verticalLayout->addWidget(detailsWidget, 3);

    // Hidden by default; shown only for manager
    detailsWidget->setVisible(false);
}

// ─── Staff view: show approved shift strings condensed into 1×7 ──────────────
void ViewSchedule_View::updateTable(const QMap<int, QList<Shift*>> &weeklyData)
{
    // 1 row × 7 cols — each cell contains all approved shifts for that day
    ui->tableSchedule->setRowCount(1);
    ui->tableSchedule->clearContents();

    for (int col = 0; col < 7; ++col)
    {
        if (!weeklyData.contains(col) || weeklyData[col].isEmpty()) {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setFlags(Qt::ItemIsEnabled);
            ui->tableSchedule->setItem(0, col, item);
            continue;
        }

        QWidget *cellWidget = new QWidget();
        QVBoxLayout *cellLayout = new QVBoxLayout(cellWidget);
        cellLayout->setContentsMargins(6, 6, 6, 6);
        cellLayout->setSpacing(6);

        for (Shift* s : weeklyData[col]) {
            QString timeStr = s->getStartTime().toString("HH:mm") + " - " + s->getEndTime().toString("HH:mm");
            
            QLabel *shiftBox = new QLabel();
            QString html = QString(
                "<div style='text-align:center;'>"
                "<div style='color:#1F2937; font-weight:bold; font-size:15px;'>%1</div>"
                "</div>"
            ).arg(timeStr);
            
            shiftBox->setText(html);
            shiftBox->setTextFormat(Qt::RichText);
            shiftBox->setAlignment(Qt::AlignCenter);
            shiftBox->setStyleSheet("QLabel { background-color: #D1FAE5; border-radius: 6px; }");
            
            cellLayout->addWidget(shiftBox);
        }

        QTableWidgetItem *bgItem = new QTableWidgetItem();
        bgItem->setBackground(Qt::white);
        ui->tableSchedule->setItem(0, col, bgItem);
        
        ui->tableSchedule->setCellWidget(0, col, cellWidget);
    }
}

// ─── Staff view: show pending (unreviewed) shifts in 1×7 ──────────────────────
void ViewSchedule_View::updatePendingTable(const QMap<int, QList<Shift*>> &weeklyData)
{
    ui->tablePending->setRowCount(1);
    ui->tablePending->clearContents();

    for (int col = 0; col < 7; ++col)
    {
        if (!weeklyData.contains(col) || weeklyData[col].isEmpty()) {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setFlags(Qt::ItemIsEnabled);
            ui->tablePending->setItem(0, col, item);
            continue;
        }

        QWidget *cellWidget = new QWidget();
        QVBoxLayout *cellLayout = new QVBoxLayout(cellWidget);
        cellLayout->setContentsMargins(6, 6, 6, 6);
        cellLayout->setSpacing(6);

        for (Shift* s : weeklyData[col]) {
            QString timeStr = s->getStartTime().toString("HH:mm") + " - " + s->getEndTime().toString("HH:mm");
            
            QString boxBgColor;
            QString textColor;
            QString statusText;

            if (s->getStatus() == 0) {
                boxBgColor = "#FEF9C3"; // Light yellow box
                textColor = "#854D0E";  // Dark yellow text
                statusText = "Chờ duyệt";
            } else {
                boxBgColor = "#FEE2E2"; // Light red box
                textColor = "#991B1B";  // Dark red text
                statusText = "Từ chối";
            }
            
            QLabel *shiftBox = new QLabel();
            QString html = QString(
                "<div style='text-align:center;'>"
                "<div style='color:#1F2937; font-weight:bold; font-size:15px;'>%1</div>"
                "<div style='color:%2; font-weight:bold; font-size:13px; margin-top:2px;'>%3</div>"
                "</div>"
            ).arg(timeStr, textColor, statusText);
            
            shiftBox->setText(html);
            shiftBox->setTextFormat(Qt::RichText);
            shiftBox->setAlignment(Qt::AlignCenter);
            shiftBox->setStyleSheet(QString(
                "QLabel { background-color: %1; border-radius: 6px; }"
            ).arg(boxBgColor));
            
            cellLayout->addWidget(shiftBox);
        }

        QTableWidgetItem *bgItem = new QTableWidgetItem();
        bgItem->setBackground(Qt::white);
        ui->tablePending->setItem(0, col, bgItem);
        
        ui->tablePending->setCellWidget(0, col, cellWidget);
    }
}

// ─── Manager view: render employee-card widgets inside each shift cell ────────
void ViewSchedule_View::updateManagerTable(const QMap<int, QMap<int, ShiftBlock *>> &gridData)
{
    ui->tableSchedule->clearContents();
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 7; ++col)
            ui->tableSchedule->removeCellWidget(row, col);

    for (int col = 0; col < 7; ++col)
    {
        if (!gridData.contains(col))
            continue;
        const QMap<int, ShiftBlock *> &dayData = gridData[col];

        for (int row = 0; row < 3; ++row)
        {
            if (!dayData.contains(row))
                continue;
            ShiftBlock *block = dayData[row];
            if (!block)
                continue;

            int count = block->getStaffCount();

            // Build the entire cell as Rich Text HTML — one QLabel per cell instead
            // of a full widget tree, which is much faster for Qt to create and paint.
            QString countBadgeStyle, cellBg;
            if (block->getStatus() == ShiftStatus::Empty)
            {
                countBadgeStyle = "background-color:#FEE2E2;color:#991B1B";
                cellBg          = "#FFF5F5";
            }
            else if (block->getStatus() == ShiftStatus::Understaffed)
            {
                countBadgeStyle = "background-color:#FEF9C3;color:#854D0E";
                cellBg          = "#FFFBEB";
            }
            else
            {
                countBadgeStyle = "background-color:#D1FAE5;color:#065F46";
                cellBg          = "#F0FDF4";
            }

            int minStaff = 0; //Config::getMinStaffPerShift();
            QString countText;
            if (count == 0) {
                countText = QString("Chưa có NV (Thiếu %1)").arg(minStaff);
            } else if (count < minStaff) {
                countText = QString("%1 NV (Thiếu %2)").arg(count).arg(minStaff - count);
            } else {
                countText = QString("Đủ %1 NV").arg(count);
            }

            QString html = QString(
                "<div style='background-color:%1;border-radius:6px;padding:4px;'>"
                "<div style='color:#6B7280;font-size:9px;font-style:italic;'>Quan ly: ---</div>"
                "<div style='%2;border-radius:4px;padding:2px 4px;"
                "font-weight:bold;font-size:11px;text-align:center;margin:2px 0;'>%3</div>")
                .arg(cellBg, countBadgeStyle, countText);

            for (User *u : block->getEmployees())
            {
                QString role = u->getRole();
                QString cardStyle = (role == "Manager" || role == "Manage")
                    ? "background-color:#EDE9FE;color:#5B21B6"
                    : "background-color:#DBEAFE;color:#1D4ED8";

                QString name = u->getName();
                if (name.length() > 14)
                    name = name.left(12) + "…";

                html += QString(
                    "<div style='%1;border-radius:4px;padding:2px 4px;"
                    "font-size:11px;font-weight:bold;margin-top:2px;'>"
                    "%2<br><span style='font-size:9px;font-weight:normal;'>%3</span></div>")
                    .arg(cardStyle, name, role.toUpper());
            }
            html += "</div>";

            QLabel *cellLabel = new QLabel(html);
            cellLabel->setTextFormat(Qt::RichText);
            cellLabel->setWordWrap(true);
            cellLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            cellLabel->setContentsMargins(2, 2, 2, 2);

            ui->tableSchedule->setCellWidget(row, col, cellLabel);
        }
    }
}

void ViewSchedule_View::updateDateRange(QDate monday)
{
    QDate sunday = monday.addDays(6);
    QString rangeText = QString("%1 - %2")
                            .arg(monday.toString("dd/MM/yyyy"))
                            .arg(sunday.toString("dd/MM/yyyy"));
    ui->lblDateRange->setText(rangeText);

    // Dynamically update the horizontal headers for all tables
    QStringList headers;
    for (int i = 0; i < 7; ++i)
    {
        QDate d = monday.addDays(i);
        int dow = d.dayOfWeek();
        QString dayName = (dow == 7) ? "Chủ Nhật" : QString("Thứ %1").arg(dow + 1);
        headers << QString("%1\n%2").arg(dayName, d.toString("dd-MM-yyyy"));
    }
    
    // The columns are already 7 columns, just set labels
    ui->tableSchedule->setHorizontalHeaderLabels(headers);
    ui->tablePending->setHorizontalHeaderLabels(headers);
}

void ViewSchedule_View::highlightToday(int currentDayIndex)
{
    // Reset all column headers
    for (int i = 0; i < 7; ++i)
    {
        QTableWidgetItem *headerItem = ui->tableSchedule->horizontalHeaderItem(i);
        if (!headerItem)
        {
            headerItem = new QTableWidgetItem();
            ui->tableSchedule->setHorizontalHeaderItem(i, headerItem);
        }
        headerItem->setBackground(QBrush());
        headerItem->setForeground(QColor(0x1F, 0x29, 0x37));
        QFont f = headerItem->font();
        f.setBold(true);
        headerItem->setFont(f);
    }

    if (currentDayIndex >= 0 && currentDayIndex <= 6)
    {
        QTableWidgetItem *todayItem = ui->tableSchedule->horizontalHeaderItem(currentDayIndex);
        if (todayItem)
        {
            todayItem->setBackground(QColor(0x2F, 0x80, 0xED));
            todayItem->setForeground(QColor(Qt::white));
            QFont f = todayItem->font();
            f.setBold(true);
            todayItem->setFont(f);
        }
    }
}

void ViewSchedule_View::updateShiftDetails(const QList<User *> &employees, const QString &timeLabel)
{
    tableShiftDetails->setRowCount(employees.size());
    tableShiftDetails->clearContents();

    if (!timeLabel.isEmpty())
    {
        lblShiftDetailTitle->setText(QString("THÔNG TIN NHÂN VIÊN - %1").arg(timeLabel.toUpper()));
    }
    else
    {
        lblShiftDetailTitle->setText("THÔNG TIN NHÂN VIÊN TRONG CA LÀM");
    }

    for (int i = 0; i < employees.size(); ++i)
    {
        User *emp = employees[i];

        QString roleDisplay = (emp->getRole() == "Staff") ? "Nhân viên" : "Quản lý";

        auto makeItem = [](const QString &text) -> QTableWidgetItem *
        {
            QTableWidgetItem *item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            item->setForeground(QBrush(QColor(0x1F, 0x29, 0x37)));
            QFont f = item->font();
            f.setBold(true);
            item->setFont(f);
            return item;
        };

        tableShiftDetails->setItem(i, 0, makeItem(QString::number(i + 1)));
        tableShiftDetails->setItem(i, 1, makeItem(QString::number(emp->getIdEmployee())));
        tableShiftDetails->setItem(i, 2, makeItem(emp->getName()));

        QTableWidgetItem *roleItem = makeItem(roleDisplay);
        roleItem->setForeground(QBrush(
            (emp->getRole() == "Staff") ? QColor(0x1D, 0x4E, 0xD8) : QColor(0x5B, 0x21, 0xB6)));
        tableShiftDetails->setItem(i, 3, roleItem);
        tableShiftDetails->setItem(i, 4, makeItem(emp->getPhoneNum()));
        // Column 5 (GIO LAM): filled by updateShiftDetailsWithTimes
        tableShiftDetails->setItem(i, 5, makeItem("---"));
    }
}

void ViewSchedule_View::updateShiftDetails(const QList<User *> &employees, const QString &timeLabel,
                                           const QMap<int, QString> &employeeTimes)
{
    // Populate rows the same way then overwrite column 5 with actual times
    updateShiftDetails(employees, timeLabel);

    for (int i = 0; i < employees.size(); ++i)
    {
        User *emp = employees[i];
        int empId = emp->getIdEmployee();
        if (employeeTimes.contains(empId))
        {
            QTableWidgetItem *timeItem = new QTableWidgetItem(employeeTimes[empId]);
            timeItem->setTextAlignment(Qt::AlignCenter);
            timeItem->setForeground(QBrush(QColor(0x1D, 0x4E, 0xD8)));
            QFont f = timeItem->font();
            f.setBold(true);
            timeItem->setFont(f);
            tableShiftDetails->setItem(i, 5, timeItem);
        }
    }
}

void ViewSchedule_View::setManagerFeaturesVisible(bool visible)
{
    if (detailsWidget)
    {
        detailsWidget->setVisible(visible);
    }

    // Hide pending section for managers — it is staff-only
    if (ui->pendingFrame)
        ui->pendingFrame->setVisible(!visible);

    // Only re-polish styles if the mode is actually changing — this is expensive
    // and causes a visible stutter when triggered on every tab navigation.
    if (m_isManagerMode == visible)
        return;
    m_isManagerMode = visible;

    // Switch table role property for styling
    ui->tableSchedule->setProperty("role", visible ? "manager" : "staff");
    ui->tableSchedule->style()->unpolish(ui->tableSchedule);
    ui->tableSchedule->style()->polish(ui->tableSchedule);

    if (visible)
    {
        // Manager mode: 3-row grid with vertical headers
        ui->tableSchedule->setRowCount(3);
        ui->tableSchedule->verticalHeader()->setVisible(true);
        ui->tableSchedule->verticalHeader()->setDefaultSectionSize(90);
        ui->tableSchedule->verticalHeader()->setMinimumWidth(110);
        QStringList vHeaders;
        for (int i = 0; i < 3; ++i)
            vHeaders << QString("%1\n%2").arg(ScheduleStyle::SHIFT_NAMES[i], ScheduleStyle::SHIFT_TIMES[i]);
        ui->tableSchedule->setVerticalHeaderLabels(vHeaders);
        ui->tableSchedule->verticalHeader()->setStyleSheet(
            "QHeaderView::section { background-color: #EFF6FF; color: #1D4ED8; font-weight: bold; "
            "border: 1px solid #BFDBFE; padding: 6px; }");
        ui->tableSchedule->setMinimumHeight(280);
    }
    else
    {
        // Staff mode: condensed 1-row grid
        ui->tableSchedule->setRowCount(1);
        ui->tableSchedule->verticalHeader()->setVisible(false);
        ui->tableSchedule->setMinimumHeight(100);
    }

    if (ui->tableSchedule->horizontalHeader())
    {
        ui->tableSchedule->horizontalHeader()->style()->unpolish(ui->tableSchedule->horizontalHeader());
        ui->tableSchedule->horizontalHeader()->style()->polish(ui->tableSchedule->horizontalHeader());
    }
}


void ViewSchedule_View::onBtnPrevClicked() { emit requestPrevWeek(); }
void ViewSchedule_View::onBtnNextClicked() { emit requestNextWeek(); }
void ViewSchedule_View::onBtnCurrentClicked() { emit requestCurrentWeek(); }
