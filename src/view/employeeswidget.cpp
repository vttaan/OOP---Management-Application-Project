#include "employeeswidget.h"
#include <QDebug>
#include <QApplication>
#include <QColor>
#include <QMouseEvent>
#include <QIcon>
#include <QSize>
#include <QPixmap>

// ============================================================
// Constructor / Destructor
// ============================================================

EmployeesWidget::EmployeesWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    populateTable();
    setupConnections();
}

EmployeesWidget::~EmployeesWidget()
{
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
    userAvatar->setStyleSheet(
        "background-color: #2563EB;"
        "color: #FFFFFF;"
        "border-radius: 18px;"
        "font-size: 12px;"
        "font-weight: bold;"
    );

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
        ":/images/dolar-svgrepo-com.svg", "#DBEAFE", "#2563EB",
        "Total Monthly Payroll", "$46.6K",
        "↑ +3.2% vs last month",
        "+3.2%", "#16A34A"
    );
    QFrame *staffCard = createMetricCard(
        ":/images/people-svgrepo-com.svg", "#DCFCE7", "#16A34A",
        "Active Staff On-Shift", "5 / 7",
        "Currently clocked in"
    );
    QFrame *absenceCard = createMetricCard(
        ":/images/warning-circle-svgrepo-com.svg", "#FEF9C3", "#CA8A04",
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
    searchRoster->setPlaceholderText("Search staff...");
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

    // Sort button
    sortBtn = new QPushButton();
    sortBtn->setObjectName("sortBtn");
    sortBtn->setIcon(QIcon(":/images/sort-vertical-svgrepo-com.svg"));
    sortBtn->setIconSize(QSize(16, 16));
    sortBtn->setFixedSize(34, 34);
    sortBtn->setToolTip("Sort");
    sortBtn->setCursor(Qt::PointingHandCursor);

    // Add Staff button
    addEmployeeBtn = new QPushButton("+ Add Staff");
    addEmployeeBtn->setObjectName("addEmployeeBtn");
    addEmployeeBtn->setFixedHeight(34);
    addEmployeeBtn->setCursor(Qt::PointingHandCursor);

    rosterHeaderLayout->addLayout(titleBlock);
    rosterHeaderLayout->addStretch();
    rosterHeaderLayout->addWidget(searchRoster);
    rosterHeaderLayout->addWidget(filterCombo);
    rosterHeaderLayout->addWidget(sortBtn);
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

void EmployeesWidget::populateTable()
{
    struct EmployeeRecord {
        QString id, name, initials, avatarColor;
        QString role, payType, monthlyRate, status;
    };

    QList<EmployeeRecord> data = {
        {"EMP-1042", "Staff A",   "SA", "#3B82F6", "Staff",   "Hourly", "$2,280/mo", "Active"},
        {"EMP-1043", "Staff B",   "SB", "#10B981", "Staff",   "Hourly", "$1,980/mo", "Active"},
        {"EMP-1044", "Staff C",   "SC", "#F59E0B", "Staff",   "Hourly", "$2,100/mo", "Absent"},
        {"EMP-1045", "Staff D",   "SD", "#EF4444", "Staff",   "Hourly", "$2,050/mo", "Active"},
        {"EMP-1046", "Manager A", "MA", "#8B5CF6", "Manager", "Salary", "$4,200/mo", "Active"},
        {"EMP-1047", "Manager B", "MB", "#6366F1", "Manager", "Salary", "$4,500/mo", "Pending"},
        {"EMP-1048", "Admin",     "AD", "#14B8A6", "Admin",   "Salary", "$3,800/mo", "Active"},
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
        // Store the role in UserRole for filtering
        idItem->setData(Qt::UserRole, emp.role);
        employeesTable->setItem(row, 0, idItem);

        // Col 1 — Name + avatar
        QWidget *nameWidget = new QWidget();
        QHBoxLayout *nameLayout = new QHBoxLayout(nameWidget);
        nameLayout->setContentsMargins(8, 4, 8, 4);
        nameLayout->setSpacing(10);
        nameLayout->addWidget(createAvatar(emp.initials, emp.avatarColor));
        QLabel *nameLabel = new QLabel(emp.name);
        nameLabel->setObjectName("empNameLabel");
        nameLabel->setFont(QFont("Segoe UI", 10, QFont::DemiBold));
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

        // Col 6 — Actions
        QWidget *actionsWidget = new QWidget();
        QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
        actionsLayout->setContentsMargins(6, 4, 6, 4);
        actionsLayout->setSpacing(5);
        actionsLayout->addWidget(createActionButton(":/images/edit-svgrepo-com.svg", "Edit"));
        actionsLayout->addWidget(createActionButton(":/images/trash-bin-trash-svgrepo-com.svg", "Delete"));
        actionsLayout->addStretch();
        employeesTable->setCellWidget(row, 6, actionsWidget);


    }
}

// ============================================================
// Connections
// ============================================================

void EmployeesWidget::setupConnections()
{
    connect(searchRoster, &QLineEdit::textChanged, this, &EmployeesWidget::filterEmployees);
    connect(filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EmployeesWidget::applyRoleFilter);
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

void EmployeesWidget::filterEmployees(const QString &searchText)
{
    const QString roleFilter = filterCombo->currentIndex() == 0
                                ? QString()
                                : filterCombo->currentText();

    for (int row = 0; row < employeesTable->rowCount(); ++row) {
        // Role filter
        QString rowRole;
        QTableWidgetItem *id0 = employeesTable->item(row, 0);
        if (id0) rowRole = id0->data(Qt::UserRole).toString();

        bool roleMatch = roleFilter.isEmpty() || rowRole == roleFilter;

        // Text filter — check item cells and name label
        bool textMatch = searchText.isEmpty();
        if (!textMatch) {
            for (int col = 0; col < employeesTable->columnCount(); ++col) {
                QTableWidgetItem *item = employeesTable->item(row, col);
                if (item && item->text().contains(searchText, Qt::CaseInsensitive)) {
                    textMatch = true;
                    break;
                }
                QWidget *w = employeesTable->cellWidget(row, col);
                if (w) {
                    QLabel *lbl = w->findChild<QLabel*>("empNameLabel");
                    if (lbl && lbl->text().contains(searchText, Qt::CaseInsensitive)) {
                        textMatch = true;
                        break;
                    }
                }
            }
        }

        employeesTable->setRowHidden(row, !(textMatch && roleMatch));
    }
}

void EmployeesWidget::applyRoleFilter(int /*index*/)
{
    // Re-run filter with current search text & new combo value
    filterEmployees(searchRoster->text());
}

void EmployeesWidget::handleAddEmployee()
{
}

// ============================================================
// Event filter — profile card click
// ============================================================

bool EmployeesWidget::eventFilter(QObject *watched, QEvent *event)
{
    QFrame *userCard = profileBlock ? profileBlock->findChild<QFrame*>("userCard") : nullptr;
    if (watched == userCard && event->type() == QEvent::MouseButtonRelease) {
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

QFrame* EmployeesWidget::createMetricCard(const QString &iconText,
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

    QLabel *iconLabel = new QLabel();
    if (iconText.startsWith(":/images/")) {
        QPixmap pix(iconText);
        iconLabel->setPixmap(pix.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        iconLabel->setText(iconText);
    }
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

QPushButton* EmployeesWidget::createActionButton(const QString &text, const QString &tooltip)
{
    QPushButton *btn = new QPushButton();
    if (text.startsWith(":/images/")) {
        btn->setIcon(QIcon(text));
        btn->setIconSize(QSize(16, 16));
        btn->setFixedSize(28, 28);
    } else {
        btn->setText(text);
    }
    btn->setObjectName("tableActionBtn");
    btn->setToolTip(tooltip);
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

