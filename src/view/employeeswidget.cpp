#include "employeeswidget.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QApplication>
#include <QColor>
#include <QMouseEvent>
#include <QMessageBox>

// ============================================================
// Constructor / Destructor
// ============================================================

EmployeesWidget::EmployeesWidget(QWidget *parent)
    : QWidget(parent)
{
    logEvent("System Event", "EmployeesWidget initialized.");
    setupUi();

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
    // PROFILE BLOCK  (replaces old top bar)
    // Shows: avatar initials | name | role badge — clickable → profile page
    // --------------------------------------------------------
    profileBlock = new QFrame();
    profileBlock->setObjectName("profileBlock");
    profileBlock->setCursor(Qt::PointingHandCursor);

    QHBoxLayout *profileLayout = new QHBoxLayout(profileBlock);
    profileLayout->setContentsMargins(0, 0, 0, 0);
    profileLayout->setSpacing(0);

    // Left: page breadcrumb title
    QLabel *lblPageTitle = new QLabel("Staff Management");
    lblPageTitle->setObjectName("lblPageTitle");

    // Right: user identity card (clickable)
    QFrame *userCard = new QFrame();
    userCard->setObjectName("userCard");
    userCard->setCursor(Qt::PointingHandCursor);
    QHBoxLayout *userCardLayout = new QHBoxLayout(userCard);
    userCardLayout->setContentsMargins(10, 6, 10, 6);
    userCardLayout->setSpacing(10);

    // Circular avatar with initials
    QLabel *userAvatar = new QLabel("ME");
    userAvatar->setObjectName("userAvatar");
    userAvatar->setFixedSize(36, 36);
    userAvatar->setAlignment(Qt::AlignCenter);

    // Name + role column
    QVBoxLayout *userInfoLayout = new QVBoxLayout();
    userInfoLayout->setSpacing(1);

    QLabel *userName = new QLabel("ME");
    userName->setObjectName("userName");

    QLabel *userRole = new QLabel("Admin");
    userRole->setObjectName("userRole");

    userInfoLayout->addWidget(userName);
    userInfoLayout->addWidget(userRole);

    // Caret icon
    QLabel *caretIcon = new QLabel("›");
    caretIcon->setObjectName("userCaret");

    userCardLayout->addWidget(userAvatar);
    userCardLayout->addLayout(userInfoLayout);
    userCardLayout->addWidget(caretIcon);

    profileLayout->addWidget(lblPageTitle);
    profileLayout->addStretch();
    profileLayout->addWidget(userCard);

    mainLayout->addWidget(profileBlock);

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
    // ROSTER CARD
    // --------------------------------------------------------
    rosterCard = new QFrame();
    rosterCard->setObjectName("rosterCard");
    rosterCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *rosterLayout = new QVBoxLayout(rosterCard);
    rosterLayout->setContentsMargins(0, 0, 0, 0);
    rosterLayout->setSpacing(0);

    // Roster header
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

    // Search bar
    searchRoster = new QLineEdit();
    searchRoster->setObjectName("searchRoster");
    searchRoster->setPlaceholderText("  🔍  Search staff...");
    searchRoster->setFixedHeight(34);
    searchRoster->setFixedWidth(190);

    // Sort / Filter combo  (###2)
    filterCombo = new QComboBox();
    filterCombo->setObjectName("filterCombo");
    filterCombo->setFixedHeight(34);
    filterCombo->addItem("All Roles");
    filterCombo->addItem("Staff");
    filterCombo->addItem("Manager");
    filterCombo->addItem("Admin");
    filterCombo->setCursor(Qt::PointingHandCursor);

    // Add Staff button
    addEmployeeBtn = new QPushButton("+ Add Staff");
    addEmployeeBtn->setObjectName("addEmployeeBtn");
    addEmployeeBtn->setFixedHeight(34);
    addEmployeeBtn->setCursor(Qt::PointingHandCursor);

    rosterHeaderLayout->addLayout(titleBlock);
    rosterHeaderLayout->addStretch();
    rosterHeaderLayout->addWidget(searchRoster);
    rosterHeaderLayout->addWidget(filterCombo);
    rosterHeaderLayout->addWidget(addEmployeeBtn);
    rosterLayout->addWidget(rosterHeader);

    // Divider
    QFrame *divider = new QFrame();
    divider->setObjectName("tableDivider");
    divider->setFrameShape(QFrame::HLine);
    rosterLayout->addWidget(divider);

    // Table
    employeesTable = new QTableWidget();
    employeesTable->setObjectName("employeesTable");
    employeesTable->setColumnCount(7);
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
    footerLabel = new QLabel("Showing 7 of 7 staff  ·  5 active, 1 absent, 1 pending");
    footerLabel->setObjectName("footerLabel");
    footerLayout->addWidget(footerLabel);
    footerLayout->addStretch();
    rosterLayout->addWidget(footerFrame);

    mainLayout->addWidget(rosterCard);
}

void EmployeesWidget::setupTableHeader()
{
    // Cols: ID | NAME | ROLE | PAY TYPE | MONTHLY RATE | STATUS | ACTIONS
    QStringList headers = {"EMPLOYEE ID", "NAME", "ROLE",
                           "PAY TYPE", "MONTHLY RATE", "STATUS", "ACTIONS"};
    employeesTable->setHorizontalHeaderLabels(headers);
    employeesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    employeesTable->setColumnWidth(0, 100);
    employeesTable->setColumnWidth(2, 88);
    employeesTable->setColumnWidth(3, 84);
    employeesTable->setColumnWidth(4, 105);
    employeesTable->setColumnWidth(5, 90);
    employeesTable->setColumnWidth(6, 80);
}

// ============================================================
// Populate Table — F&B simplified names, monthly salaries
// ============================================================

// ============================================================
// loadEmployees — slot called by Controller to update the table
// ============================================================

void EmployeesWidget::loadEmployees(const QList<User*> &employees)
{
    QStringList avatarColors = {
        "#3B82F6","#10B981","#F59E0B","#EF4444",
        "#8B5CF6","#6366F1","#14B8A6"
    };

    employeesTable->clearContents();
    employeesTable->setRowCount(employees.size());

    for (int row = 0; row < employees.size(); ++row) {
        User *emp = employees[row];
        employeesTable->setRowHeight(row, 50);

        QString colorHex = avatarColors[row % avatarColors.size()];
        QString initials  = emp->getName().left(2).toUpper();

        // Col 0 — ID
        QTableWidgetItem *idItem = new QTableWidgetItem(
            QString("EMP-%1").arg(emp->getIdEmployee()));
        idItem->setForeground(QColor("#64748B"));
        idItem->setFont(QFont("Segoe UI", 9));
        idItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        idItem->setData(Qt::UserRole,     emp->getRole());
        idItem->setData(Qt::UserRole + 1, emp->getIdEmployee());
        employeesTable->setItem(row, 0, idItem);

        // Col 1 — Name + avatar
        QWidget *nameWidget = new QWidget();
        QHBoxLayout *nameLayout = new QHBoxLayout(nameWidget);
        nameLayout->setContentsMargins(8, 4, 8, 4); nameLayout->setSpacing(10);
        nameLayout->addWidget(createAvatar(initials, colorHex));
        QLabel *nameLabel = new QLabel(emp->getName());
        nameLabel->setObjectName("empNameLabel");
        nameLabel->setFont(QFont("Segoe UI", 10, QFont::DemiBold));
        nameLayout->addWidget(nameLabel);
        nameLayout->addStretch();
        employeesTable->setCellWidget(row, 1, nameWidget);

        // Col 2 — Role badge
        QWidget *roleWidget = new QWidget();
        QHBoxLayout *roleLayout = new QHBoxLayout(roleWidget);
        roleLayout->setContentsMargins(8, 4, 8, 4);
        roleLayout->addWidget(createRoleBadge(emp->getRole()));
        roleLayout->addStretch();
        employeesTable->setCellWidget(row, 2, roleWidget);

        // Col 3 — Pay Type
        QString payType = (emp->getRole() == "Staff") ? "Hourly" : "Salary";
        QTableWidgetItem *payItem = new QTableWidgetItem(payType);
        payItem->setForeground(QColor("#374151"));
        payItem->setFont(QFont("Segoe UI", 9));
        payItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        employeesTable->setItem(row, 3, payItem);

        // Col 4 — Monthly Rate
        QString rateStr = QString("$%1/mo")
            .arg(QString::number(emp->getSalary(), 'f', 0));
        QTableWidgetItem *rateItem = new QTableWidgetItem(rateStr);
        rateItem->setForeground(QColor("#0F172A"));
        rateItem->setFont(QFont("Segoe UI", 9, QFont::DemiBold));
        rateItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        employeesTable->setItem(row, 4, rateItem);

        // Col 5 — Status
        QWidget *statusWidget = new QWidget();
        QHBoxLayout *statusLayout = new QHBoxLayout(statusWidget);
        statusLayout->setContentsMargins(8, 4, 8, 4);
        statusLayout->addWidget(createStatusBadge("Active"));
        statusLayout->addStretch();
        employeesTable->setCellWidget(row, 5, statusWidget);

        // Col 6 — Actions — only emits signal, does not call model
        QWidget *actionsWidget = new QWidget();
        QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
        actionsLayout->setContentsMargins(6, 4, 6, 4); actionsLayout->setSpacing(5);

        QPushButton *editBtn   = createActionButton("✏️", "Edit");
        QPushButton *deleteBtn = createActionButton("🗑️", "Delete");

        int empId = emp->getIdEmployee();
        // View only emits — Controller will handle
        connect(editBtn, &QPushButton::clicked, this,
                [this, empId]() { emit requestEditEmployee(empId); });
        connect(deleteBtn, &QPushButton::clicked, this,
                [this, empId]() { emit requestDeleteEmployee(empId); });

        actionsLayout->addWidget(editBtn);
        actionsLayout->addWidget(deleteBtn);
        actionsLayout->addStretch();
        employeesTable->setCellWidget(row, 6, actionsWidget);
    }

    footerLabel->setText(QString("Showing %1 employee(s)").arg(employees.size()));
    logEvent("UI Data Load", QString("Table refreshed with %1 rows.").arg(employees.size()));
}

void EmployeesWidget::showError(const QString &msg)
{
    QMessageBox::warning(this, "Error", msg);
}

void EmployeesWidget::showSuccess(const QString &msg)
{
    QMessageBox::information(this, "Success", msg);
}

// ============================================================
// Connections
// ============================================================

void EmployeesWidget::setupConnections()
{
    connect(addEmployeeBtn, &QPushButton::clicked, this, &EmployeesWidget::handleAddEmployee);

    // Profile block click — forward to Dashboard_View via signal
    // We install an event filter on the userCard child
    QFrame *userCard = profileBlock->findChild<QFrame*>("userCard");
    if (userCard) {
        userCard->installEventFilter(this);
    }
}




// ============================================================
// Slots
// ============================================================


void EmployeesWidget::handleAddEmployee()
{
    // View only emits signal — Controller will show dialog and call model
    logEvent("System Event", "Add Staff button clicked — emitting requestAddEmployee.");
    emit requestAddEmployee();
}

// ============================================================
// Event filter — profile card click
// ============================================================

bool EmployeesWidget::eventFilter(QObject *watched, QEvent *event)
{
    QFrame *userCard = profileBlock ? profileBlock->findChild<QFrame*>("userCard") : nullptr;
    if (watched == userCard && event->type() == QEvent::MouseButtonRelease) {
        logEvent("Navigation", "Profile block clicked — emitting profileClicked.");
        emit profileClicked();
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

// ============================================================
// Widget Factories
// ============================================================

QLabel* EmployeesWidget::createAvatar(const QString &initials, const QString &bgColor)
{
    QLabel *avatar = new QLabel(initials);
    avatar->setObjectName("empAvatar");
    avatar->setFixedSize(32, 32);
    avatar->setAlignment(Qt::AlignCenter);
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
        badgeLabel->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 10px;").arg(badgeColor));
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

QLabel* EmployeesWidget::createStatusBadge(const QString &status)
{
    QLabel *badge = new QLabel(status);
    badge->setAlignment(Qt::AlignCenter);
    badge->setFixedHeight(20);

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
