#include "employeeswidget.h"
#include <QSqlDatabase>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QApplication>
#include <QSpacerItem>
#include <QColor>

// ============================================================
// Constructor / Destructor
// ============================================================

EmployeesWidget::EmployeesWidget(QWidget *parent)
    : QWidget(parent)
{
    logEvent("System Event", "EmployeesWidget initialized.");
    setupUi();
    populateTable();
    setupConnections();
    logEvent("System Event", "EmployeesWidget setup completed.");
}

EmployeesWidget::~EmployeesWidget()
{
    logEvent("System Event", "EmployeesWidget destroyed.");
}

// ============================================================
// UI Construction
// ============================================================

void EmployeesWidget::setupUi()
{
    setObjectName("EmployeesWidget");

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 16, 24, 16);
    mainLayout->setSpacing(16);

    // --------------------------------------------------------
    // TOP BAR  —  breadcrumb | search | clock | bell
    // --------------------------------------------------------
    QFrame *topBarFrame = new QFrame();
    topBarFrame->setObjectName("topBarFrame");
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBarFrame);
    topBarLayout->setContentsMargins(0, 0, 0, 0);
    topBarLayout->setSpacing(12);

    lblBreadcrumb = new QLabel("CorpHQ  ›  Staff");
    lblBreadcrumb->setObjectName("lblBreadcrumb");

    searchTopBar = new QLineEdit();
    searchTopBar->setObjectName("searchTopBar");
    searchTopBar->setPlaceholderText("  Quick search…  ⌘K");
    searchTopBar->setFixedHeight(34);
    searchTopBar->setFixedWidth(210);

    lblClock = new QLabel();
    lblClock->setObjectName("lblClock");
    clockTimer = new QTimer(this);
    connect(clockTimer, &QTimer::timeout, this, &EmployeesWidget::updateClock);
    clockTimer->start(1000);
    updateClock();

    // Bell with badge
    QFrame *bellFrame = new QFrame();
    bellFrame->setObjectName("bellFrame");
    bellFrame->setFixedSize(36, 36);
    QLabel *bellIcon = new QLabel("🔔", bellFrame);
    bellIcon->setObjectName("bellIcon");
    bellIcon->setGeometry(0, 0, 36, 36);
    bellIcon->setAlignment(Qt::AlignCenter);
    QLabel *bellBadge = new QLabel("2", bellFrame);
    bellBadge->setObjectName("bellBadge");
    bellBadge->setGeometry(21, 1, 15, 15);
    bellBadge->setAlignment(Qt::AlignCenter);

    topBarLayout->addWidget(lblBreadcrumb);
    topBarLayout->addStretch();
    topBarLayout->addWidget(searchTopBar);
    topBarLayout->addWidget(lblClock);
    topBarLayout->addWidget(bellFrame);

    mainLayout->addWidget(topBarFrame);

    // --------------------------------------------------------
    // METRICS CARDS  (3-column)
    // --------------------------------------------------------
    metricsLayout = new QHBoxLayout();
    metricsLayout->setSpacing(14);

    QFrame *payrollCard = createMetricCard(
        "💰", "#DBEAFE", "#2563EB",
        "Total Monthly Payroll", "$46.6K",
        "↑ +3.2% vs last month",
        "+3.2%", "#16A34A"
    );
    QFrame *staffCard = createMetricCard(
        "👥", "#DCFCE7", "#16A34A",
        "Active Staff On-Shift", "5 / 7",
        "Currently clocked in"
    );
    QFrame *absenceCard = createMetricCard(
        "⚠️", "#FEF9C3", "#CA8A04",
        "Pending Absences", "2",
        "1 absent · 1 pending approval"
    );

    metricsLayout->addWidget(payrollCard);
    metricsLayout->addWidget(staffCard);
    metricsLayout->addWidget(absenceCard);

    mainLayout->addLayout(metricsLayout);

    // --------------------------------------------------------
    // CONTENT ROW  —  Roster table (left) + Info panels (right)
    // --------------------------------------------------------
    contentRowLayout = new QHBoxLayout();
    contentRowLayout->setSpacing(14);

    // ---- LEFT: Roster Card ----
    rosterCard = new QFrame();
    rosterCard->setObjectName("rosterCard");
    rosterCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *rosterLayout = new QVBoxLayout(rosterCard);
    rosterLayout->setContentsMargins(0, 0, 0, 0);
    rosterLayout->setSpacing(0);

    // Roster header row
    QFrame *rosterHeader = new QFrame();
    rosterHeader->setObjectName("rosterHeader");
    QHBoxLayout *rosterHeaderLayout = new QHBoxLayout(rosterHeader);
    rosterHeaderLayout->setContentsMargins(18, 14, 18, 14);
    rosterHeaderLayout->setSpacing(10);

    QVBoxLayout *titleBlock = new QVBoxLayout();
    titleBlock->setSpacing(2);
    QLabel *rosterTitle = new QLabel("Staff Roster");
    rosterTitle->setObjectName("rosterTitle");
    QLabel *rosterSubtitle = new QLabel("7 employees total");
    rosterSubtitle->setObjectName("rosterSubtitle");
    titleBlock->addWidget(rosterTitle);
    titleBlock->addWidget(rosterSubtitle);

    searchRoster = new QLineEdit();
    searchRoster->setObjectName("searchRoster");
    searchRoster->setPlaceholderText("  Search staff...");
    searchRoster->setFixedHeight(34);
    searchRoster->setFixedWidth(190);

    addEmployeeBtn = new QPushButton("+ Add Staff");
    addEmployeeBtn->setObjectName("addEmployeeBtn");
    addEmployeeBtn->setFixedHeight(34);
    addEmployeeBtn->setCursor(Qt::PointingHandCursor);

    rosterHeaderLayout->addLayout(titleBlock);
    rosterHeaderLayout->addStretch();
    rosterHeaderLayout->addWidget(searchRoster);
    rosterHeaderLayout->addWidget(addEmployeeBtn);
    rosterLayout->addWidget(rosterHeader);

    // Divider line
    QFrame *divider = new QFrame();
    divider->setObjectName("tableDivider");
    divider->setFrameShape(QFrame::HLine);
    rosterLayout->addWidget(divider);

    // Table
    employeesTable = new QTableWidget();
    employeesTable->setObjectName("employeesTable");
    employeesTable->setColumnCount(7); // removed Department col
    employeesTable->verticalHeader()->setVisible(false);
    employeesTable->setShowGrid(false);
    employeesTable->setAlternatingRowColors(true);
    employeesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    employeesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    employeesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    employeesTable->setFocusPolicy(Qt::NoFocus);
    employeesTable->horizontalHeader()->setStretchLastSection(true);
    employeesTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    employeesTable->horizontalHeader()->setHighlightSections(false);
    employeesTable->setFrameShape(QFrame::NoFrame);

    setupTableHeader();
    rosterLayout->addWidget(employeesTable);

    // Footer
    QFrame *footerFrame = new QFrame();
    footerFrame->setObjectName("tableFooterFrame");
    QHBoxLayout *footerLayout = new QHBoxLayout(footerFrame);
    footerLayout->setContentsMargins(18, 8, 18, 8);
    footerLabel = new QLabel("Showing 7 of 7 staff · 5 active, 1 absent, 1 pending");
    footerLabel->setObjectName("footerLabel");
    footerLayout->addWidget(footerLabel);
    footerLayout->addStretch();
    rosterLayout->addWidget(footerFrame);

    contentRowLayout->addWidget(rosterCard, 3); // stretch factor 3

    // ---- RIGHT: Info Panels column ----
    QVBoxLayout *rightPanelColumn = new QVBoxLayout();
    rightPanelColumn->setSpacing(14);

    // Next Shift panel
    nextShiftPanel = createInfoPanel(
        "Next Shift",
        "🗓️",
        "nextShiftPanel"
    );

    // Absent Today panel
    absentTodayPanel = createInfoPanel(
        "Absent Today",
        "🚫",
        "absentTodayPanel"
    );

    rightPanelColumn->addWidget(nextShiftPanel, 1);
    rightPanelColumn->addWidget(absentTodayPanel, 1);

    contentRowLayout->addLayout(rightPanelColumn, 1); // stretch factor 1

    mainLayout->addLayout(contentRowLayout);
}

void EmployeesWidget::setupTableHeader()
{
    // Cols: ID | NAME | ROLE | PAY TYPE | MONTHLY RATE | STATUS | ACTIONS
    QStringList headers = {"EMPLOYEE ID", "NAME", "ROLE",
                           "PAY TYPE", "MONTHLY RATE", "STATUS", "ACTIONS"};
    employeesTable->setHorizontalHeaderLabels(headers);

    // NAME column stretches
    employeesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    employeesTable->setColumnWidth(0, 100); // ID
    employeesTable->setColumnWidth(2, 88);  // ROLE
    employeesTable->setColumnWidth(3, 84);  // PAY TYPE
    employeesTable->setColumnWidth(4, 105); // MONTHLY RATE
    employeesTable->setColumnWidth(5, 90);  // STATUS
    employeesTable->setColumnWidth(6, 80);  // ACTIONS
}

// ============================================================
// Populate Table  —  F&B simplified names, monthly salaries
// ============================================================

void EmployeesWidget::populateTable()
{
    // Cols: 0=ID, 1=Name+Avatar, 2=Role, 3=PayType, 4=MonthlyRate, 5=Status, 6=Actions
    struct EmployeeRecord {
        QString id, name, initials, avatarColor;
        QString role, payType, monthlyRate, status;
    };

    QList<EmployeeRecord> data = {
        {"EMP-1042", "Staff A",     "SA", "#3B82F6", "Staff",   "Hourly",  "$2,280/mo",  "Active"},
        {"EMP-1043", "Staff B",     "SB", "#10B981", "Staff",   "Hourly",  "$1,980/mo",  "Active"},
        {"EMP-1044", "Staff C",     "SC", "#F59E0B", "Staff",   "Hourly",  "$2,100/mo",  "Absent"},
        {"EMP-1045", "Staff D",     "SD", "#EF4444", "Staff",   "Hourly",  "$2,050/mo",  "Active"},
        {"EMP-1046", "Manager A",   "MA", "#8B5CF6", "Manager", "Salary",  "$4,200/mo",  "Active"},
        {"EMP-1047", "Manager B",   "MB", "#6366F1", "Manager", "Salary",  "$4,500/mo",  "Pending"},
        {"EMP-1048", "Admin",       "AD", "#14B8A6", "Admin",   "Salary",  "$3,800/mo",  "Active"},
    };

    employeesTable->setRowCount(data.size());

    for (int row = 0; row < data.size(); ++row) {
        const auto &emp = data[row];
        employeesTable->setRowHeight(row, 50);

        // Col 0 — Employee ID
        QTableWidgetItem *idItem = new QTableWidgetItem(emp.id);
        idItem->setForeground(QColor("#64748B"));
        idItem->setFont(QFont("Segoe UI", 9));
        idItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        employeesTable->setItem(row, 0, idItem);

        // Col 1 — Name with circular avatar
        QWidget *nameWidget = new QWidget();
        QHBoxLayout *nameLayout = new QHBoxLayout(nameWidget);
        nameLayout->setContentsMargins(8, 4, 8, 4);
        nameLayout->setSpacing(10);
        QLabel *avatar = createAvatar(emp.initials, emp.avatarColor);
        QLabel *nameLabel = new QLabel(emp.name);
        nameLabel->setObjectName("empNameLabel");
        nameLabel->setFont(QFont("Segoe UI", 10, QFont::DemiBold));
        nameLayout->addWidget(avatar);
        nameLayout->addWidget(nameLabel);
        nameLayout->addStretch();
        employeesTable->setCellWidget(row, 1, nameWidget);

        // Col 2 — Role badge
        QWidget *roleWidget = new QWidget();
        QHBoxLayout *roleLayout = new QHBoxLayout(roleWidget);
        roleLayout->setContentsMargins(8, 4, 8, 4);
        roleLayout->addWidget(createRoleBadge(emp.role));
        roleLayout->addStretch();
        employeesTable->setCellWidget(row, 2, roleWidget);

        // Col 3 — Pay Type
        QTableWidgetItem *payItem = new QTableWidgetItem(emp.payType);
        payItem->setForeground(QColor("#374151"));
        payItem->setFont(QFont("Segoe UI", 9));
        payItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        employeesTable->setItem(row, 3, payItem);

        // Col 4 — Monthly Rate
        QTableWidgetItem *rateItem = new QTableWidgetItem(emp.monthlyRate);
        rateItem->setForeground(QColor("#0F172A"));
        rateItem->setFont(QFont("Segoe UI", 9, QFont::DemiBold));
        rateItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        employeesTable->setItem(row, 4, rateItem);

        // Col 5 — Status badge
        QWidget *statusWidget = new QWidget();
        QHBoxLayout *statusLayout = new QHBoxLayout(statusWidget);
        statusLayout->setContentsMargins(8, 4, 8, 4);
        statusLayout->addWidget(createStatusBadge(emp.status));
        statusLayout->addStretch();
        employeesTable->setCellWidget(row, 5, statusWidget);

        // Col 6 — Actions (edit + delete)
        QWidget *actionsWidget = new QWidget();
        QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
        actionsLayout->setContentsMargins(6, 4, 6, 4);
        actionsLayout->setSpacing(5);
        actionsLayout->addWidget(createActionButton("✏️", "Edit"));
        actionsLayout->addWidget(createActionButton("🗑️", "Delete"));
        actionsLayout->addStretch();
        employeesTable->setCellWidget(row, 6, actionsWidget);

        logEvent("UI Data Load", QString("Loaded staff row %1: %2").arg(row + 1).arg(emp.name));
    }
}

// ============================================================
// Connections
// ============================================================

void EmployeesWidget::setupConnections()
{
    connect(searchRoster, &QLineEdit::textChanged, this, &EmployeesWidget::filterEmployees);
    connect(addEmployeeBtn, &QPushButton::clicked, this, &EmployeesWidget::handleAddEmployee);
}

// ============================================================
// Slots
// ============================================================

void EmployeesWidget::filterEmployees(const QString &searchText)
{
    logEvent("UI Data Fetch", QString("Filtering staff by: '%1'").arg(searchText));
    for (int row = 0; row < employeesTable->rowCount(); ++row) {
        bool match = false;
        for (int col = 0; col < employeesTable->columnCount(); ++col) {
            QTableWidgetItem *item = employeesTable->item(row, col);
            if (item && item->text().contains(searchText, Qt::CaseInsensitive)) {
                match = true;
                break;
            }
            QWidget *w = employeesTable->cellWidget(row, col);
            if (w) {
                QLabel *lbl = w->findChild<QLabel*>("empNameLabel");
                if (lbl && lbl->text().contains(searchText, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }
            }
        }
        employeesTable->setRowHidden(row, !match && !searchText.isEmpty());
    }
}

void EmployeesWidget::handleAddEmployee()
{
    logEvent("System Event", "Add Staff button clicked.");
}

void EmployeesWidget::updateClock()
{
    QDateTime now = QDateTime::currentDateTime();
    lblClock->setText(now.toString("ddd, MMM d  hh:mm AP"));
}

// ============================================================
// Helper Widget Factories
// ============================================================

QLabel* EmployeesWidget::createAvatar(const QString &initials, const QString &bgColor)
{
    QLabel *avatar = new QLabel(initials);
    avatar->setObjectName("empAvatar");
    avatar->setFixedSize(32, 32);
    avatar->setAlignment(Qt::AlignCenter);
    // Inline style needed here because QSS cannot address per-instance background colors
    avatar->setStyleSheet(QString(
        "background-color: %1;"
        "color: #FFFFFF;"
        "border-radius: 16px;"
        "font-size: 11px;"
        "font-weight: bold;"
    ).arg(bgColor));
    return avatar;
}

QFrame* EmployeesWidget::createMetricCard(const QString &iconEmoji,
                                           const QString &iconBg,
                                           const QString &iconColor,
                                           const QString &title,
                                           const QString &value,
                                           const QString &subtitle,
                                           const QString &badge,
                                           const QString &badgeColor)
{
    QFrame *card = new QFrame();
    card->setObjectName("metricCard");
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    card->setMinimumHeight(108);

    QHBoxLayout *cardLayout = new QHBoxLayout(card);
    cardLayout->setContentsMargins(16, 14, 16, 14);
    cardLayout->setSpacing(14);

    // Circular icon  — inline style needed for per-card colour
    QLabel *iconLabel = new QLabel(iconEmoji);
    iconLabel->setObjectName("metricIcon");
    iconLabel->setFixedSize(50, 50);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet(QString(
        "background-color: %1;"
        "color: %2;"
        "border-radius: 25px;"
        "font-size: 20px;"
    ).arg(iconBg, iconColor));

    // Text block
    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setSpacing(3);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setObjectName("metricTitle");

    QLabel *valueLabel = new QLabel(value);
    valueLabel->setObjectName("metricValue");

    QHBoxLayout *subRow = new QHBoxLayout();
    subRow->setSpacing(5);
    QLabel *subtitleLabel = new QLabel(subtitle);
    subtitleLabel->setObjectName("metricSubtitle");
    subRow->addWidget(subtitleLabel);
    if (!badge.isEmpty()) {
        QLabel *badgeLabel = new QLabel(badge);
        badgeLabel->setStyleSheet(QString(
            "color: %1; font-weight: bold; font-size: 10px;").arg(badgeColor));
        subRow->addWidget(badgeLabel);
    }
    subRow->addStretch();

    textLayout->addWidget(titleLabel);
    textLayout->addWidget(valueLabel);
    textLayout->addLayout(subRow);

    cardLayout->addWidget(iconLabel);
    cardLayout->addLayout(textLayout);
    cardLayout->addStretch();

    return card;
}

// Creates a labelled info panel (Next Shift / Absent Today)
// with an icon, title, and empty body space ready for data.
QFrame* EmployeesWidget::createInfoPanel(const QString &title,
                                          const QString &iconEmoji,
                                          const QString &objectName)
{
    QFrame *panel = new QFrame();
    panel->setObjectName(objectName);
    panel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    panel->setMinimumWidth(220);

    QVBoxLayout *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(0, 0, 0, 0);
    panelLayout->setSpacing(0);

    // Panel header bar
    QFrame *headerBar = new QFrame();
    headerBar->setObjectName(objectName + QString("Header"));
    QHBoxLayout *headerLayout = new QHBoxLayout(headerBar);
    headerLayout->setContentsMargins(14, 12, 14, 12);
    headerLayout->setSpacing(8);

    QLabel *iconLbl = new QLabel(iconEmoji);
    iconLbl->setObjectName("infoPanelIcon");
    iconLbl->setFixedSize(24, 24);
    iconLbl->setAlignment(Qt::AlignCenter);

    QLabel *titleLbl = new QLabel(title);
    titleLbl->setObjectName("infoPanelTitle");

    headerLayout->addWidget(iconLbl);
    headerLayout->addWidget(titleLbl);
    headerLayout->addStretch();
    panelLayout->addWidget(headerBar);

    // Thin separator
    QFrame *sep = new QFrame();
    sep->setObjectName("infoPanelDivider");
    sep->setFrameShape(QFrame::HLine);
    panelLayout->addWidget(sep);

    // Body — empty placeholder space
    QFrame *body = new QFrame();
    body->setObjectName("infoPanelBody");
    body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(14, 14, 14, 14);
    bodyLayout->setSpacing(10);

    // Placeholder hint label
    QLabel *hintLbl = new QLabel("— No data —");
    hintLbl->setObjectName("infoPanelHint");
    hintLbl->setAlignment(Qt::AlignCenter);
    bodyLayout->addStretch();
    bodyLayout->addWidget(hintLbl);
    bodyLayout->addStretch();

    panelLayout->addWidget(body, 1);

    return panel;
}

QLabel* EmployeesWidget::createStatusBadge(const QString &status)
{
    QLabel *badge = new QLabel(status);
    badge->setAlignment(Qt::AlignCenter);
    badge->setFixedHeight(20);

    // Inline styles — per-status colours cannot come from named-object QSS
    QString style;
    if (status == "Active")
        style = "background-color:#DCFCE7;color:#15803D;border-radius:10px;font-size:10px;font-weight:bold;padding:1px 9px;";
    else if (status == "Absent")
        style = "background-color:#FEE2E2;color:#DC2626;border-radius:10px;font-size:10px;font-weight:bold;padding:1px 9px;";
    else
        style = "background-color:#FEF3C7;color:#D97706;border-radius:10px;font-size:10px;font-weight:bold;padding:1px 9px;";

    badge->setStyleSheet(style);
    return badge;
}

QLabel* EmployeesWidget::createRoleBadge(const QString &role)
{
    QLabel *badge = new QLabel(role);
    badge->setAlignment(Qt::AlignCenter);
    badge->setFixedHeight(20);

    // Inline styles — per-role colours cannot come from named-object QSS
    QString style;
    if (role == "Manager")
        style = "background-color:#EDE9FE;color:#6D28D9;border-radius:10px;font-size:10px;font-weight:bold;padding:1px 9px;";
    else if (role == "Admin")
        style = "background-color:#DBEAFE;color:#1D4ED8;border-radius:10px;font-size:10px;font-weight:bold;padding:1px 9px;";
    else
        style = "background-color:#F1F5F9;color:#475569;border-radius:10px;font-size:10px;font-weight:bold;padding:1px 9px;";

    badge->setStyleSheet(style);
    return badge;
}

QPushButton* EmployeesWidget::createActionButton(const QString &emoji, const QString &tooltip)
{
    QPushButton *btn = new QPushButton(emoji);
    btn->setObjectName("tableActionBtn");
    btn->setToolTip(tooltip);
    btn->setFixedSize(28, 28);
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

// ============================================================
// Logging
// ============================================================

void EmployeesWidget::logEvent(const QString &eventType, const QString &details)
{
    QString logFileName = "ai-logs/25127052_AIUsageLog.md";
    QFile logFile(logFileName);
    bool isNewFile = !logFile.exists();

    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        if (isNewFile) {
            out << "# Application Activity Log — EmployeesWidget\n\n";
            out << "| Timestamp | Event Type | Details |\n";
            out << "|---|---|---|\n";
        }
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        QString safe = details;
        safe.replace("|", "\\|");
        safe.replace("\n", " ");
        out << "| " << timestamp << " | **" << eventType << "** | " << safe << " |\n";
        logFile.close();
    } else {
        qWarning() << "Failed to open log file:" << logFileName;
    }
}
