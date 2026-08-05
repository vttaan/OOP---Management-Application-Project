#include "ViewSchedule_View.h"
#include "ui_ViewSchedule_View.h"
#include "core/ShiftBlock.h"
#include "core/User.h"
#include "core/Shift.h"
#include "utils/ScheduleDTOs.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace
{
    QString displayRoleName(const QString &role)
    {
        if (role.compare("Manager", Qt::CaseInsensitive) == 0 ||
            role.compare("Manage", Qt::CaseInsensitive) == 0)
            return QString::fromUtf8("Quản lý");
        if (role.compare("Cashier", Qt::CaseInsensitive) == 0)
            return QString::fromUtf8("Thu ngân");
        if (role.compare("HallStaff", Qt::CaseInsensitive) == 0)
            return QString::fromUtf8("Nhân viên sảnh");
        if (role.compare("KitchenAssistant", Qt::CaseInsensitive) == 0)
            return QString::fromUtf8("Phụ bếp");
        if (role.compare("Admin", Qt::CaseInsensitive) == 0)
            return QString::fromUtf8("Quản trị viên");
        return role;
    }
}

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

    managerViewMode = new QComboBox(ui->scheduleHeader);
    managerViewMode->addItems({"Tổng quan", "Chi tiết"});
    managerRoleFilter = new QComboBox(ui->scheduleHeader);
    managerRoleFilter->addItems(
        {"Tất cả vai trò", "Quản lý", "Thu ngân", "Nhân viên sảnh", "Phụ bếp"});
    const QString managerFilterStyle =
        "QComboBox {"
        "   background-color: #F9FAFB;"
        "   border: 1px solid #D1D5DB;"
        "   border-radius: 6px;"
        "   padding: 4px 8px;"
        "   min-height: 24px;"
        "   color: #1F2937;"
        "}"
        "QComboBox:hover {"
        "   border: 1px solid #1a73e8;"
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
        "QComboBox QAbstractItemView {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #D1D5DB;"
        "   border-radius: 4px;"
        "   outline: none;"
        "}"
        "QComboBox QAbstractItemView::item {"
        "   padding: 8px 12px;"
        "   min-height: 24px;"
        "}"
        "QComboBox QAbstractItemView::item:hover, QComboBox QAbstractItemView::item:selected {"
        "   background-color: #1a73e8;"
        "   color: white;"
        "}";
    managerViewMode->setStyleSheet(managerFilterStyle);
    managerRoleFilter->setStyleSheet(managerFilterStyle);
    managerViewMode->setVisible(false);
    managerRoleFilter->setVisible(false);
    ui->headerLayout->insertWidget(ui->headerLayout->count() - 1, managerViewMode);
    ui->headerLayout->insertWidget(ui->headerLayout->count() - 1, managerRoleFilter);
    connect(managerViewMode, qOverload<int>(&QComboBox::currentIndexChanged), this,
            [this](int)
            { if (!m_managerGrid.isEmpty()) updateManagerTable(m_managerGrid); });
    connect(managerRoleFilter, qOverload<int>(&QComboBox::currentIndexChanged), this,
            [this](int)
            {
                if (!m_managerGrid.isEmpty())
                    updateManagerTable(m_managerGrid);
                int row = ui->tableSchedule->currentRow();
                int col = ui->tableSchedule->currentColumn();
                if (row >= 0 && col >= 0)
                    emit shiftClicked(row, col);
            });

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
        "border: 1px solid #E2E8F0; "
        "border-radius: 14px; "
        "}"
        "QLabel { background: transparent; border: none; }");

    QVBoxLayout *detailsLayout = new QVBoxLayout(detailsWidget);
    detailsLayout->setContentsMargins(20, 16, 20, 18);
    detailsLayout->setSpacing(12);

    QHBoxLayout *detailHeader = new QHBoxLayout();
    detailHeader->setSpacing(12);
    QVBoxLayout *detailHeading = new QVBoxLayout();
    detailHeading->setSpacing(2);
    lblShiftDetailTitle = new QLabel("Thông tin nhân viên", detailsWidget);
    lblShiftDetailTitle->setStyleSheet(
        "font-size:16px;font-weight:800;color:#0F172A;");
    lblShiftDetailSubtitle = new QLabel(
        "Chọn một ca để xem danh sách nhân sự đã được xếp.", detailsWidget);
    lblShiftDetailSubtitle->setStyleSheet(
        "font-size:11px;color:#64748B;");
    detailHeading->addWidget(lblShiftDetailTitle);
    detailHeading->addWidget(lblShiftDetailSubtitle);
    lblShiftDetailCount = new QLabel("0 nhân viên", detailsWidget);
    lblShiftDetailCount->setAlignment(Qt::AlignCenter);
    lblShiftDetailCount->setMinimumWidth(92);
    lblShiftDetailCount->setStyleSheet(
        "background:#EFF6FF;color:#1D4ED8;border:1px solid #BFDBFE;"
        "border-radius:13px;padding:5px 10px;font-size:11px;font-weight:700;");
    detailHeader->addLayout(detailHeading, 1);
    detailHeader->addWidget(lblShiftDetailCount, 0, Qt::AlignVCenter);

    tableShiftDetails = new QTableWidget(detailsWidget);
    tableShiftDetails->setColumnCount(5);
    tableShiftDetails->setHorizontalHeaderLabels(
        {"NHÂN VIÊN", "VAI TRÒ", "LIÊN HỆ", "GIỜ LÀM", "HÀNH ĐỘNG"});
    tableShiftDetails->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tableShiftDetails->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tableShiftDetails->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    tableShiftDetails->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    tableShiftDetails->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    tableShiftDetails->horizontalHeader()->setMinimumSectionSize(120);
    tableShiftDetails->horizontalHeader()->setFixedHeight(38);
    tableShiftDetails->verticalHeader()->setVisible(false);
    tableShiftDetails->verticalHeader()->setDefaultSectionSize(54);
    tableShiftDetails->setSelectionMode(QAbstractItemView::NoSelection);
    tableShiftDetails->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableShiftDetails->setFocusPolicy(Qt::NoFocus);
    tableShiftDetails->setShowGrid(false);
    tableShiftDetails->setWordWrap(false);
    tableShiftDetails->setStyleSheet(
        "QTableWidget { background:#FFFFFF;border:1px solid #E2E8F0;"
        "border-radius:10px;color:#334155;gridline-color:transparent; }"
        "QHeaderView::section { background:#F8FAFC;color:#64748B;font-size:10px;"
        "font-weight:700;padding:8px 12px;border:none;border-bottom:1px solid #E2E8F0; }"
        "QTableWidget::item { padding:8px 12px;border:none;"
        "border-bottom:1px solid #F1F5F9;color:#334155; }"
        "QTableWidget::item:alternate { background:#F8FAFC; }");
    tableShiftDetails->setAlternatingRowColors(true);

    detailsLayout->addLayout(detailHeader);
    detailsLayout->addWidget(tableShiftDetails);

    ui->verticalLayout->addWidget(detailsWidget, 3);

    // Hidden by default; shown only for manager
    detailsWidget->setVisible(false);
}

// ─── Staff view: show approved shift strings condensed into 1×7 ──────────────
void ViewSchedule_View::updateTable(const QMap<int, QList<Shift *>> &weeklyData)
{
    // 1 row × 7 cols — each cell contains all approved shifts for that day
    ui->tableSchedule->setRowCount(1);
    ui->tableSchedule->clearContents();

    for (int col = 0; col < 7; ++col)
    {
        if (!weeklyData.contains(col) || weeklyData[col].isEmpty())
        {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setFlags(Qt::ItemIsEnabled);
            ui->tableSchedule->setItem(0, col, item);
            continue;
        }

        QWidget *cellWidget = new QWidget();
        QVBoxLayout *cellLayout = new QVBoxLayout(cellWidget);
        cellLayout->setContentsMargins(6, 6, 6, 6);
        cellLayout->setSpacing(6);

        for (Shift *s : weeklyData[col])
        {
            QString timeStr = s->getStartTime().toString("HH:mm") + " - " + s->getEndTime().toString("HH:mm");

            QLabel *shiftBox = new QLabel();
            QString html = QString(
                               "<div style='text-align:center;'>"
                               "<div style='color:#1F2937; font-weight:bold; font-size:15px;'>%1</div>"
                               "</div>")
                               .arg(timeStr);

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
void ViewSchedule_View::updatePendingTable(const QMap<int, QList<Shift *>> &weeklyData)
{
    ui->tablePending->setRowCount(1);
    ui->tablePending->clearContents();

    for (int col = 0; col < 7; ++col)
    {
        if (!weeklyData.contains(col) || weeklyData[col].isEmpty())
        {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setFlags(Qt::ItemIsEnabled);
            ui->tablePending->setItem(0, col, item);
            continue;
        }

        QWidget *cellWidget = new QWidget();
        QVBoxLayout *cellLayout = new QVBoxLayout(cellWidget);
        cellLayout->setContentsMargins(6, 6, 6, 6);
        cellLayout->setSpacing(6);

        for (Shift *s : weeklyData[col])
        {
            QString timeStr = s->getStartTime().toString("HH:mm") + " - " + s->getEndTime().toString("HH:mm");

            QString boxBgColor;
            QString textColor;
            QString statusText;

            if (s->getStatus() == 0)
            {
                boxBgColor = "#FEF9C3"; // Light yellow box
                textColor = "#854D0E";  // Dark yellow text
                statusText = "Chờ duyệt";
            }
            else
            {
                boxBgColor = "#FEE2E2"; // Light red box
                textColor = "#991B1B";  // Dark red text
                statusText = "Từ chối";
            }

            QLabel *shiftBox = new QLabel();
            QString html = QString(
                               "<div style='text-align:center;'>"
                               "<div style='color:#1F2937; font-weight:bold; font-size:15px;'>%1</div>"
                               "<div style='color:%2; font-weight:bold; font-size:13px; margin-top:2px;'>%3</div>"
                               "</div>")
                               .arg(timeStr, textColor, statusText);

            shiftBox->setText(html);
            shiftBox->setTextFormat(Qt::RichText);
            shiftBox->setAlignment(Qt::AlignCenter);
            shiftBox->setStyleSheet(QString(
                                        "QLabel { background-color: %1; border-radius: 6px; }")
                                        .arg(boxBgColor));

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
    m_managerGrid = gridData;
    QString selectedRole;
    switch (managerRoleFilter ? managerRoleFilter->currentIndex() : 0)
    {
    case 1:
        selectedRole = "Manager";
        break;
    case 2:
        selectedRole = "Cashier";
        break;
    case 3:
        selectedRole = "HallStaff";
        break;
    case 4:
        selectedRole = "KitchenAssistant";
        break;
    default:
        break;
    }
    const bool overviewMode = !managerViewMode || managerViewMode->currentIndex() == 0;
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

            QList<User *> visibleEmployees;
            for (User *employee : block->getEmployees())
            {
                QString role = employee->getRole();
                bool matches = selectedRole.isEmpty() ||
                               role.compare(selectedRole, Qt::CaseInsensitive) == 0 ||
                               (selectedRole == "Manager" && role.compare("Manage", Qt::CaseInsensitive) == 0);
                if (matches)
                    visibleEmployees.append(employee);
            }
            int count = visibleEmployees.size();

            // Build the entire cell as Rich Text HTML — one QLabel per cell instead
            // of a full widget tree, which is much faster for Qt to create and paint.
            QString countBadgeStyle, cellBg;
            if (block->getStatus() == ShiftStatus::Empty)
            {
                countBadgeStyle = "background-color:#FEE2E2;color:#991B1B";
                cellBg = "#FFF5F5";
            }
            else if (block->getStatus() == ShiftStatus::Understaffed)
            {
                countBadgeStyle = "background-color:#FEF9C3;color:#854D0E";
                cellBg = "#FFFBEB";
            }
            else
            {
                countBadgeStyle = "background-color:#D1FAE5;color:#065F46";
                cellBg = "#F0FDF4";
            }

            QString countText;
            if (count == 0)
                countText = selectedRole.isEmpty() ? "Chưa có nhân viên"
                                                   : "Không có vai trò này";
            else
                countText = QString("%1 nhân viên").arg(count);

            QString html = QString(
                               "<div style='background-color:%1;border-radius:6px;padding:5px;'>"
                               "<div style='%2;border-radius:4px;padding:2px 4px;"
                               "font-weight:bold;font-size:11px;text-align:center;margin:2px 0;'>%3</div>")
                               .arg(cellBg, countBadgeStyle, countText);

            if (overviewMode)
            {
                QMap<QString, int> roleCounts;
                for (User *employee : visibleEmployees)
                    roleCounts[employee->getRole()]++;
                for (auto it = roleCounts.constBegin(); it != roleCounts.constEnd(); ++it)
                    html += QString(
                                "<div style='background:#FFFFFF;color:#334155;border:1px solid #E2E8F0;"
                                "border-radius:4px;padding:4px 6px;font-size:12px;margin-top:3px;'>"
                                "%1 <b>%2</b></div>")
                                .arg(displayRoleName(it.key()))
                                .arg(it.value());
            }
            else
            {
                for (User *u : visibleEmployees)
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
                                "font-size:12px;font-weight:bold;margin-top:2px;'>"
                                "%2<br><span style='font-size:10px;font-weight:normal;'>%3</span></div>")
                                .arg(cardStyle, name, displayRoleName(role));
                }
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

void ViewSchedule_View::updateShiftDetails(const QList<User *> &employees, const QList<int> &shiftIds, const QString &timeLabel)
{
    QString selectedRole;
    switch (managerRoleFilter ? managerRoleFilter->currentIndex() : 0)
    {
    case 1:
        selectedRole = "Manager";
        break;
    case 2:
        selectedRole = "Cashier";
        break;
    case 3:
        selectedRole = "HallStaff";
        break;
    case 4:
        selectedRole = "KitchenAssistant";
        break;
    default:
        break;
    }
    QList<User *> visibleEmployees;
    for (User *employee : employees)
    {
        QString role = employee->getRole();
        if (selectedRole.isEmpty() || role.compare(selectedRole, Qt::CaseInsensitive) == 0 ||
            (selectedRole == "Manager" && role.compare("Manage", Qt::CaseInsensitive) == 0))
            visibleEmployees.append(employee);
    }
    tableShiftDetails->clearSpans();
    tableShiftDetails->setRowCount(qMax(1, visibleEmployees.size()));
    tableShiftDetails->clearContents();

    if (!timeLabel.isEmpty())
    {
        lblShiftDetailSubtitle->setText(
            QString("Ca làm %1 · Danh sách nhân sự đã công bố").arg(timeLabel));
    }
    else
    {
        lblShiftDetailSubtitle->setText(
            "Chọn một ca để xem danh sách nhân sự đã được xếp.");
    }
    lblShiftDetailCount->setText(QString("%1 nhân viên").arg(visibleEmployees.size()));

    if (visibleEmployees.isEmpty())
    {
        tableShiftDetails->setSpan(0, 0, 1, tableShiftDetails->columnCount());
        QTableWidgetItem *emptyItem = new QTableWidgetItem(
            timeLabel.isEmpty()
                ? "Chưa chọn ca làm"
                : "Không có nhân viên phù hợp với bộ lọc hiện tại");
        emptyItem->setTextAlignment(Qt::AlignCenter);
        emptyItem->setForeground(QBrush(QColor("#94A3B8")));
        tableShiftDetails->setItem(0, 0, emptyItem);
        tableShiftDetails->setRowHeight(0, 64);
        return;
    }

    for (int i = 0; i < visibleEmployees.size(); ++i)
    {
        User *emp = visibleEmployees[i];

        QString roleDisplay = emp->getRole();
        roleDisplay = displayRoleName(roleDisplay);

        auto makeItem = [](const QString &text, Qt::Alignment alignment = Qt::AlignVCenter | Qt::AlignLeft) -> QTableWidgetItem *
        {
            QTableWidgetItem *item = new QTableWidgetItem(text);
            item->setTextAlignment(alignment);
            item->setForeground(QBrush(QColor(0x1F, 0x29, 0x37)));
            return item;
        };

        QTableWidgetItem *employeeItem = makeItem(emp->getName());
        employeeItem->setData(Qt::UserRole, emp->getIdEmployee());
        QFont employeeFont = employeeItem->font();
        employeeFont.setBold(true);
        employeeItem->setFont(employeeFont);
        employeeItem->setToolTip(QString("Mã nhân viên: %1").arg(emp->getIdEmployee()));
        tableShiftDetails->setItem(i, 0, employeeItem);

        QWidget *roleCell = new QWidget(tableShiftDetails);
        roleCell->setStyleSheet("background:transparent;");
        QHBoxLayout *roleLayout = new QHBoxLayout(roleCell);
        roleLayout->setContentsMargins(10, 7, 10, 7);
        QLabel *roleBadge = new QLabel(roleDisplay, roleCell);
        const bool isManager = emp->getRole() == "Manager" || emp->getRole() == "Manage";
        roleBadge->setAlignment(Qt::AlignCenter);
        roleBadge->setStyleSheet(
            isManager
                ? "background:#F3E8FF;color:#6D28D9;border-radius:11px;padding:4px 9px;font-weight:700;"
                : "background:#DBEAFE;color:#1D4ED8;border-radius:11px;padding:4px 9px;font-weight:700;");
        roleLayout->addWidget(roleBadge, 0, Qt::AlignLeft | Qt::AlignVCenter);
        roleLayout->addStretch();
        tableShiftDetails->setCellWidget(i, 1, roleCell);

        tableShiftDetails->setItem(
            i, 2, makeItem(emp->getPhoneNum(), Qt::AlignCenter));
        QTableWidgetItem *timeItem = makeItem(
            timeLabel.isEmpty() ? "Chưa cập nhật" : timeLabel, Qt::AlignCenter);
        timeItem->setForeground(QBrush(QColor("#1D4ED8")));
        QFont timeFont = timeItem->font();
        timeFont.setBold(true);
        timeItem->setFont(timeFont);
        tableShiftDetails->setItem(i, 3, timeItem);
        tableShiftDetails->setRowHeight(i, 54);
    }
}

void ViewSchedule_View::updateShiftDetails(const QList<User *> &employees, const QList<int> &shiftIds, const QString &timeLabel,
                                           const QMap<int, QString> &employeeTimes)
{
    // Populate rows the same way then overwrite column 4 with actual times
    updateShiftDetails(employees, shiftIds, timeLabel);

    for (int i = 0; i < tableShiftDetails->rowCount(); ++i)
    {
        QTableWidgetItem *employeeItem = tableShiftDetails->item(i, 0);
        if (!employeeItem)
            continue;
        int empId = employeeItem->data(Qt::UserRole).toInt();
        if (employeeTimes.contains(empId))
        {
            QTableWidgetItem *timeItem = new QTableWidgetItem(employeeTimes[empId]);
            timeItem->setTextAlignment(Qt::AlignCenter);
            timeItem->setForeground(QBrush(QColor(0x1D, 0x4E, 0xD8)));
            QFont f = timeItem->font();
            f.setBold(true);
            timeItem->setFont(f);
            // tableShiftDetails->setItem(i, 4, timeItem);
            tableShiftDetails->setItem(i, 3, timeItem);
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
    if (managerViewMode)
        managerViewMode->setVisible(visible);
    if (managerRoleFilter)
        managerRoleFilter->setVisible(visible);
    ui->scheduleTitle->setText(visible ? "Lịch làm việc toàn bộ nhân sự"
                                       : "Lịch làm việc cá nhân");

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

void ViewSchedule_View::showReplacementDialog(int oldShiftId, const QList<PendingShiftInfo> &replacements)
{
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("Thay thế nhân viên");
    dlg->setMinimumWidth(600);
    dlg->setMinimumHeight(400);
    dlg->setStyleSheet("QDialog { background-color: #F8FAFC; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(dlg);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(12);

    QLabel *title = new QLabel("CHỌN NHÂN VIÊN THAY THẾ", dlg);
    title->setStyleSheet("font-size: 14px; font-weight: bold; color: #1F2937;");
    mainLayout->addWidget(title);

    if (replacements.isEmpty())
    {
        QLabel *lblEmpty = new QLabel("Không có nhân viên nào phù hợp (bị từ chối) trong cùng khung giờ.", dlg);
        lblEmpty->setStyleSheet("color: #6B7280; font-style: italic;");
        mainLayout->addWidget(lblEmpty);

        QPushButton *btnClose = new QPushButton("Đóng", dlg);
        btnClose->setStyleSheet(
            "QPushButton { background-color: #E5E7EB; color: #374151; "
            "border-radius: 4px; padding: 6px 12px; font-weight: bold; } "
            "QPushButton:hover { background-color: #D1D5DB; }");
        connect(btnClose, &QPushButton::clicked, dlg, &QDialog::accept);
        mainLayout->addWidget(btnClose, 0, Qt::AlignRight);

        dlg->exec();
        dlg->deleteLater();
        return;
    }

    QTableWidget *tbl = new QTableWidget(replacements.size(), 3, dlg);
    tbl->setHorizontalHeaderLabels({"ID", "TÊN NHÂN VIÊN", "HÀNH ĐỘNG"});
    tbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tbl->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tbl->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tbl->setSelectionMode(QAbstractItemView::NoSelection);
    tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tbl->setFocusPolicy(Qt::NoFocus);
    tbl->setAlternatingRowColors(true);
    tbl->verticalHeader()->setVisible(false);
    tbl->setStyleSheet(
        "QTableWidget { background-color: #FFFFFF; border: 1px solid #E5E7EB; border-radius: 8px; }"
        "QHeaderView::section { background-color: #2F80ED; color: white; font-weight: bold; padding: 7px; border: none; }"
        "QTableWidget::item { padding: 6px; color: #1F2937; }"
        "QTableWidget::item:alternate { background-color: #F0F9FF; }");

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

    for (int i = 0; i < replacements.size(); ++i)
    {
        const PendingShiftInfo &info = replacements[i];

        tbl->setRowHeight(i, 42);
        tbl->setItem(i, 0, makeItem(QString::number(info.employeeId)));
        tbl->setItem(i, 1, makeItem(info.employeeName));

        QWidget *actionCell = new QWidget();
        QHBoxLayout *actionLayout = new QHBoxLayout(actionCell);
        actionLayout->setContentsMargins(4, 2, 4, 2);

        QPushButton *btnSelect = new QPushButton("Chọn", actionCell);
        btnSelect->setStyleSheet(
            "QPushButton { background-color: #219653; color: white; border-radius: 4px; "
            "padding: 4px 12px; font-weight: bold; font-size: 11px; } "
            "QPushButton:hover { background-color: #1E824C; }");

        int newShiftId = info.shiftId;
        connect(btnSelect, &QPushButton::clicked, this, [this, oldShiftId, newShiftId, dlg]()
                {
            emit requestConfirmReplacement(oldShiftId, newShiftId);
            dlg->accept(); });

        actionLayout->addWidget(btnSelect);
        tbl->setCellWidget(i, 2, actionCell);
    }

    mainLayout->addWidget(tbl);

    QPushButton *btnClose = new QPushButton("Hủy", dlg);
    btnClose->setStyleSheet(
        "QPushButton { background-color: #E5E7EB; color: #374151; "
        "border-radius: 4px; padding: 6px 12px; font-weight: bold; } "
        "QPushButton:hover { background-color: #D1D5DB; }");
    connect(btnClose, &QPushButton::clicked, dlg, &QDialog::reject);
    mainLayout->addWidget(btnClose, 0, Qt::AlignRight);

    dlg->exec();
    dlg->deleteLater();
}
