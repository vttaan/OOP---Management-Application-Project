#include "employeeswidget.h"
#include <QDebug>
#include <QApplication>
#include <QColor>
#include <QMouseEvent>
#include <QIcon>
#include <QSize>
#include <QPixmap>
#include <QMessageBox>

// ============================================================
// Constructor / Destructor
// ============================================================

EmployeesWidget::EmployeesWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    buildFilterDropdown();
    buildSortDropdown();
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
    // PROFILE BLOCK
    // --------------------------------------------------------
    profileBlock = new QFrame();
    profileBlock->setObjectName("profileBlock");
    profileBlock->setCursor(Qt::PointingHandCursor);

    QHBoxLayout *profileLayout = new QHBoxLayout(profileBlock);
    profileLayout->setContentsMargins(0, 0, 0, 0);
    profileLayout->setSpacing(0);

    // Left: page breadcrumb title
    QLabel *lblPageTitle = new QLabel("Quản lý nhân viên");
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

    QLabel *userRole = new QLabel("Quản trị viên");
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
    // METRICS CARDS (3-column)
    // --------------------------------------------------------
    metricsLayout = new QHBoxLayout();
    metricsLayout->setSpacing(14);

    QFrame *payrollCard = createMetricCard(
        ":/images/dolar-svgrepo-com.svg", "#DBEAFE", "#2563EB",
        "Tổng bảng lương tháng", "$46.6K",
        "↑ +3.2% so với tháng trước",
        "+3.2%", "#16A34A");
    QFrame *staffCard = createMetricCard(
        ":/images/people-svgrepo-com.svg", "#DCFCE7", "#16A34A",
        "Nhân viên đang làm việc", "5 / 7",
        "Hiện đang trong ca");
    QFrame *absenceCard = createMetricCard(
        ":/images/warning-circle-svgrepo-com.svg", "#FEF9C3", "#CA8A04",
        "Vắng mặt chờ duyệt", "2",
        "1 vắng · 1 chờ phê duyệt");

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

    // ---- Roster header ----
    QFrame *rosterHeader = new QFrame();
    rosterHeader->setObjectName("rosterHeader");
    QHBoxLayout *rosterHeaderLayout = new QHBoxLayout(rosterHeader);
    rosterHeaderLayout->setContentsMargins(18, 14, 18, 14);
    rosterHeaderLayout->setSpacing(10);

    QVBoxLayout *titleBlock = new QVBoxLayout();
    titleBlock->setSpacing(2);
    QLabel *rosterTitle = new QLabel("Danh sách nhân viên");
    rosterTitle->setObjectName("rosterTitle");
    QLabel *rosterSubtitle = new QLabel("Tổng cộng 7 nhân viên");
    rosterSubtitle->setObjectName("rosterSubtitle");
    titleBlock->addWidget(rosterTitle);
    titleBlock->addWidget(rosterSubtitle);

    // Search bar
    searchRoster = new QLineEdit();
    searchRoster->setObjectName("searchRoster");
    searchRoster->setPlaceholderText("Tìm kiếm nhân viên...");
    searchRoster->setFixedHeight(34);
    searchRoster->setFixedWidth(190);

    // Filter button (icon only — replaces old filterCombo)
    filterBtn = new QPushButton();
    filterBtn->setObjectName("filterBtn");
    filterBtn->setIcon(QIcon(":/images/filter.svg"));
    filterBtn->setIconSize(QSize(16, 16));
    filterBtn->setFixedSize(34, 34);
    filterBtn->setToolTip("Lọc");
    filterBtn->setCursor(Qt::PointingHandCursor);

    // Sort button
    sortBtn = new QPushButton();
    sortBtn->setObjectName("sortBtn");
    sortBtn->setIcon(QIcon(":/images/sort-vertical-svgrepo-com.svg"));
    sortBtn->setIconSize(QSize(16, 16));
    sortBtn->setFixedSize(34, 34);
    sortBtn->setToolTip("Sắp xếp");
    sortBtn->setCursor(Qt::PointingHandCursor);

    // Add Staff button
    addEmployeeBtn = new QPushButton("+ Thêm nhân viên");
    addEmployeeBtn->setObjectName("addEmployeeBtn");
    addEmployeeBtn->setFixedHeight(34);
    addEmployeeBtn->setCursor(Qt::PointingHandCursor);

    rosterHeaderLayout->addLayout(titleBlock);
    rosterHeaderLayout->addStretch();
    rosterHeaderLayout->addWidget(searchRoster);
    rosterHeaderLayout->addWidget(filterBtn);
    rosterHeaderLayout->addWidget(sortBtn);
    rosterHeaderLayout->addWidget(addEmployeeBtn);
    rosterLayout->addWidget(rosterHeader);

    // Header divider
    QFrame *divider = new QFrame();
    divider->setObjectName("tableDivider");
    divider->setFrameShape(QFrame::HLine);
    rosterLayout->addWidget(divider);

    // ---- Sort Tag Bar (hidden until a sort field is chosen) ----
    sortTagBar = new QFrame();
    sortTagBar->setObjectName("sortTagBar");
    QHBoxLayout *sortTagLayout = new QHBoxLayout(sortTagBar);
    sortTagLayout->setContentsMargins(18, 6, 18, 6);
    sortTagLayout->setSpacing(8);

    QLabel *sortTagPrefix = new QLabel("Sắp xếp:");
    sortTagPrefix->setObjectName("sortTagPrefix");

    sortTagLabel = new QLabel();
    sortTagLabel->setObjectName("sortTagLabel");
    sortTagLabel->setCursor(Qt::PointingHandCursor);

    sortTagLayout->addWidget(sortTagPrefix);
    sortTagLayout->addWidget(sortTagLabel);
    sortTagLayout->addStretch();

    sortTagBar->hide();
    rosterLayout->addWidget(sortTagBar);

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
    footerLabel = new QLabel("Hiển thị 7 / 7 nhân viên  ·  5 đang làm, 1 vắng, 1 chờ duyệt");
    footerLabel->setObjectName("footerLabel");
    footerLayout->addWidget(footerLabel);
    footerLayout->addStretch();
    rosterLayout->addWidget(footerFrame);

    mainLayout->addWidget(rosterCard);
}

void EmployeesWidget::setupTableHeader()
{
    // Cols: MÃ NV | TÊN | VAI TRÒ | LOẠI LƯƠNG | MỨC LƯƠNG | TRẠNG THÁI | THAO TÁC
    QStringList headers = {"MÃ NHÂN VIÊN", "TÊN", "VAI TRÒ",
                           "LOẠI LƯƠNG", "MỨC LƯƠNG", "TRẠNG THÁI", "THAO TÁC"};
    employeesTable->setHorizontalHeaderLabels(headers);
    employeesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    employeesTable->setColumnWidth(0, 100);
    employeesTable->setColumnWidth(2, 88);
    employeesTable->setColumnWidth(3, 84);
    employeesTable->setColumnWidth(4, 105);
    employeesTable->setColumnWidth(5, 100);
    employeesTable->setColumnWidth(6, 80);
}

// ============================================================
// Build Filter Dropdown (floating child widget)
// ============================================================

void EmployeesWidget::buildFilterDropdown()
{
    filterDropdown = new QFrame(this);
    filterDropdown->setObjectName("filterDropdown");
    filterDropdown->setFrameShape(QFrame::StyledPanel);
    filterDropdown->raise();
    filterDropdown->hide();

    QVBoxLayout *layout = new QVBoxLayout(filterDropdown);
    layout->setContentsMargins(14, 10, 14, 12);
    layout->setSpacing(6);

    // --- Role section ---
    QLabel *lblRole = new QLabel("Vai trò");
    lblRole->setObjectName("filterSectionLabel");
    layout->addWidget(lblRole);

    chkStaff   = new QCheckBox("Nhân viên");
    chkManager = new QCheckBox("Quản lý");
    chkAdmin   = new QCheckBox("Quản trị viên");
    layout->addWidget(chkStaff);
    layout->addWidget(chkManager);
    layout->addWidget(chkAdmin);

    // Separator
    QFrame *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setObjectName("filterSeparator");
    layout->addWidget(sep);

    // --- Gender section ---
    QLabel *lblGender = new QLabel("Giới tính");
    lblGender->setObjectName("filterSectionLabel");
    layout->addWidget(lblGender);

    chkMale   = new QCheckBox("Nam");
    chkFemale = new QCheckBox("Nữ");
    layout->addWidget(chkMale);
    layout->addWidget(chkFemale);

    filterDropdown->adjustSize();
}

// ============================================================
// Build Sort Dropdown (floating child widget)
// ============================================================

void EmployeesWidget::buildSortDropdown()
{
    sortDropdown = new QFrame(this);
    sortDropdown->setObjectName("sortDropdown");
    sortDropdown->setFrameShape(QFrame::StyledPanel);
    sortDropdown->raise();
    sortDropdown->hide();

    QVBoxLayout *layout = new QVBoxLayout(sortDropdown);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(4);

    QPushButton *btnSortId   = new QPushButton("Sắp xếp theo Mã NV");
    QPushButton *btnSortName = new QPushButton("Sắp xếp theo Tên");

    btnSortId->setObjectName("sortDropdownItem");
    btnSortName->setObjectName("sortDropdownItem");
    btnSortId->setFlat(true);
    btnSortName->setFlat(true);
    btnSortId->setCursor(Qt::PointingHandCursor);
    btnSortName->setCursor(Qt::PointingHandCursor);

    layout->addWidget(btnSortId);
    layout->addWidget(btnSortName);

    sortDropdown->adjustSize();

    connect(btnSortId,   &QPushButton::clicked, this, [this]() { onSortFieldSelected("id"); });
    connect(btnSortName, &QPushButton::clicked, this, [this]() { onSortFieldSelected("name"); });
}

// ============================================================
// Connections
// ============================================================

void EmployeesWidget::setupConnections()
{
    // Add button
    connect(addEmployeeBtn, &QPushButton::clicked, this, &EmployeesWidget::handleAddEmployee);

    // Search bar — emit combined update when text changes
    connect(searchRoster, &QLineEdit::textChanged, this, &EmployeesWidget::emitUpdateRequest);

    // Filter dropdown toggle
    connect(filterBtn, &QPushButton::clicked, this, &EmployeesWidget::toggleFilterDropdown);

    // Filter checkboxes — emit combined update on any change
    connect(chkStaff,   &QCheckBox::checkStateChanged, this, &EmployeesWidget::emitUpdateRequest);
    connect(chkManager, &QCheckBox::checkStateChanged, this, &EmployeesWidget::emitUpdateRequest);
    connect(chkAdmin,   &QCheckBox::checkStateChanged, this, &EmployeesWidget::emitUpdateRequest);
    connect(chkMale,    &QCheckBox::checkStateChanged, this, &EmployeesWidget::emitUpdateRequest);
    connect(chkFemale,  &QCheckBox::checkStateChanged, this, &EmployeesWidget::emitUpdateRequest);

    // Sort dropdown toggle
    connect(sortBtn, &QPushButton::clicked, this, &EmployeesWidget::toggleSortDropdown);

    // Sort tag label click via event filter
    sortTagLabel->installEventFilter(this);

    // Profile block click — event filter on userCard
    QFrame *userCard = profileBlock->findChild<QFrame *>("userCard");
    if (userCard)
        userCard->installEventFilter(this);
}

// ============================================================
// loadEmployees — called by Controller to update the table
// ============================================================

void EmployeesWidget::loadEmployees(const QList<User *> &employees)
{
    // The controller already applied filter→search→sort before calling us;
    // just render what we received.
    renderTable(employees);
}

void EmployeesWidget::renderTable(const QList<User *> &employees)
{
    QStringList avatarColors = {
        "#3B82F6", "#10B981", "#F59E0B", "#EF4444",
        "#8B5CF6", "#6366F1", "#14B8A6"};

    employeesTable->clearContents();
    employeesTable->setRowCount(employees.size());

    for (int row = 0; row < employees.size(); ++row)
    {
        User *emp = employees[row];
        employeesTable->setRowHeight(row, 50);

        QString colorHex = avatarColors[row % avatarColors.size()];
        QString initials = emp->getName().left(2).toUpper();

        // Col 0 — ID  (stores role, id, gender in UserRole data — Bug 5 fix & new)
        QTableWidgetItem *idItem = new QTableWidgetItem(
            QString("NV-%1").arg(emp->getIdEmployee()));
        idItem->setForeground(QColor("#64748B"));
        idItem->setFont(QFont("Segoe UI", 9));
        idItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        idItem->setData(Qt::UserRole,     emp->getRole());
        idItem->setData(Qt::UserRole + 1, emp->getIdEmployee());
        idItem->setData(Qt::UserRole + 2, emp->getGender()); // store gender for filter
        employeesTable->setItem(row, 0, idItem);

        // Col 1 — Name + avatar
        QWidget *nameWidget = new QWidget();
        QHBoxLayout *nameLayout = new QHBoxLayout(nameWidget);
        nameLayout->setContentsMargins(8, 4, 8, 4);
        nameLayout->setSpacing(10);
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
        QString payType = (emp->getRole() == "Staff") ? "Theo giờ" : "Cố định";
        QTableWidgetItem *payItem = new QTableWidgetItem(payType);
        payItem->setForeground(QColor("#374151"));
        payItem->setFont(QFont("Segoe UI", 9));
        payItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        employeesTable->setItem(row, 3, payItem);

        // Col 4 — Monthly Rate
        QString rateStr = QString("$%1/th")
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
        statusLayout->addWidget(createStatusBadge("Đang làm"));
        statusLayout->addStretch();
        employeesTable->setCellWidget(row, 5, statusWidget);

        // Col 6 — Actions — Bug 6 fix: connect signals with captured empId
        int empId = emp->getIdEmployee();
        QWidget *actionsWidget = new QWidget();
        QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
        actionsLayout->setContentsMargins(6, 4, 6, 4);
        actionsLayout->setSpacing(5);

        QPushButton *editBtn = createActionButton(":/images/edit-svgrepo-com.svg", "Chỉnh sửa");
        QPushButton *delBtn  = createActionButton(":/images/trash-bin-trash-svgrepo-com.svg", "Xóa");

        connect(editBtn, &QPushButton::clicked, this, [this, empId]() {
            emit requestEditEmployee(empId);
        });
        connect(delBtn, &QPushButton::clicked, this, [this, empId]() {
            emit requestDeleteEmployee(empId);
        });

        actionsLayout->addWidget(editBtn);
        actionsLayout->addWidget(delBtn);
        actionsLayout->addStretch();
        employeesTable->setCellWidget(row, 6, actionsWidget);
    }
}

// ============================================================
// showError / showSuccess — Bug 4 fix
// ============================================================

void EmployeesWidget::showError(const QString &msg)
{
    QMessageBox::critical(this, "Lỗi", msg);
}

void EmployeesWidget::showSuccess(const QString &msg)
{
    QMessageBox::information(this, "Thành công", msg);
}

// ============================================================
// emitUpdateRequest — collects all active criteria and signals the Controller
// ============================================================

void EmployeesWidget::emitUpdateRequest()
{
    QString searchText = searchRoster->text();

    QList<QString> contentFilter;
    if (chkStaff->isChecked())   contentFilter << "Staff";
    if (chkManager->isChecked()) contentFilter << "Manage";
    if (chkAdmin->isChecked())   contentFilter << "Admin";
    if (chkMale->isChecked())   contentFilter <<  "Nam";
    if (chkFemale->isChecked()) contentFilter <<  "Nữ";

    // change code, if have many field to sort
    QList<QString> contentSort;
    if(!m_sortField.isEmpty()) contentSort.append(m_sortField);

    emit requestUpdate(searchText, contentFilter, contentSort, m_sortDir);
}

void EmployeesWidget::toggleFilterDropdown()
{
    if (m_filterOpen) {
        filterDropdown->hide();
        filterBtn->setIcon(QIcon(":/images/filter.svg"));
        m_filterOpen = false;
    } else {
        // Position below filterBtn
        QPoint pos = filterBtn->mapTo(this, QPoint(0, filterBtn->height() + 2));
        filterDropdown->adjustSize();
        filterDropdown->move(pos);
        filterDropdown->raise();
        filterDropdown->show();
        filterBtn->setIcon(QIcon(":/images/filter-slash.svg"));
        m_filterOpen = true;
    }
}

// ============================================================
// Sort Slots
// ============================================================

void EmployeesWidget::toggleSortDropdown()
{
    if (m_sortOpen) {
        sortDropdown->hide();
        m_sortOpen = false;
    } else {
        QPoint pos = sortBtn->mapTo(this, QPoint(0, sortBtn->height() + 2));
        sortDropdown->adjustSize();
        sortDropdown->move(pos);
        sortDropdown->raise();
        sortDropdown->show();
        m_sortOpen = true;
    }
}

void EmployeesWidget::onSortFieldSelected(const QString &field)
{
    // Close dropdown
    sortDropdown->hide();
    m_sortOpen = false;

    m_sortField = field;
    m_sortDir   = 1; // ascending initially

    // Update sort button icon
    sortBtn->setIcon(QIcon(":/images/sort-from-bottom-to-top.svg"));

    // Show sort tag chip
    QString fieldName = (field == "id") ? "Mã NV" : "Tên";
    sortTagLabel->setText(QString("📌 Theo %1: từ thấp đến cao  ✕").arg(fieldName));
    sortTagBar->show();

    // Ask the Controller to sort (it will filter+search+sort and call loadEmployees)
    emitUpdateRequest();
}

void EmployeesWidget::cycleSortDirection()
{
    if (m_sortField.isEmpty()) return;

    if (m_sortDir == 1) {
        // ascending → descending
        m_sortDir = -1;
        sortBtn->setIcon(QIcon(":/images/sort-from-top-to-bottom.svg"));
        QString fieldName = (m_sortField == "id") ? "Mã NV" : "Tên";
        sortTagLabel->setText(QString("📌 Theo %1: từ cao đến thấp  ✕").arg(fieldName));
    } else {
        // descending → reset (remove sort)
        m_sortDir   = 0;
        m_sortField = "";
        sortBtn->setIcon(QIcon(":/images/sort-vertical-svgrepo-com.svg"));
        sortTagBar->hide();
    }

    // Ask the Controller to re-apply all criteria with new sort direction
    emitUpdateRequest();
}

// ============================================================
// Slot — handleAddEmployee
// ============================================================

void EmployeesWidget::handleAddEmployee()
{
    emit requestAddEmployee();
}

// ============================================================
// Event filter — profile card click + sort tag click
// ============================================================

bool EmployeesWidget::eventFilter(QObject *watched, QEvent *event)
{
    // Profile card click
    QFrame *userCard = profileBlock ? profileBlock->findChild<QFrame *>("userCard") : nullptr;
    if (watched == userCard && event->type() == QEvent::MouseButtonRelease)
    {
        emit profileClicked();
        return true;
    }

    // Sort tag label click → cycle sort direction
    if (watched == sortTagLabel && event->type() == QEvent::MouseButtonRelease)
    {
        cycleSortDirection();
        return true;
    }

    return QWidget::eventFilter(watched, event);
}

// ============================================================
// Widget Factories
// ============================================================

QLabel *EmployeesWidget::createAvatar(const QString &initials, const QString &bgColor)
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
                              "font-weight: bold;")
                              .arg(bgColor));
    return avatar;
}

QFrame *EmployeesWidget::createMetricCard(const QString &iconText,
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
    if (iconText.startsWith(":/images/"))
    {
        QPixmap pix(iconText);
        iconLabel->setPixmap(pix.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else
    {
        iconLabel->setText(iconText);
    }
    iconLabel->setObjectName("metricIcon");
    iconLabel->setFixedSize(50, 50);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet(QString(
                                 "background-color: %1;"
                                 "color: %2;"
                                 "border-radius: 25px;"
                                 "font-size: 20px;")
                                 .arg(iconBg, iconColor));

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
    if (!badge.isEmpty())
    {
        QLabel *badgeLabel = new QLabel(badge);
        badgeLabel->setStyleSheet(
            QString("color: %1; font-weight: bold; font-size: 10px;").arg(badgeColor));
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

QLabel *EmployeesWidget::createStatusBadge(const QString &status)
{
    QLabel *badge = new QLabel(status);
    badge->setAlignment(Qt::AlignCenter);
    badge->setFixedHeight(20);

    QString style;
    if (status == "Đang làm" || status == "Active")
        style = "background-color:#DCFCE7;color:#15803D;border-radius:10px;"
                "font-size:10px;font-weight:bold;padding:1px 9px;";
    else if (status == "Vắng" || status == "Absent")
        style = "background-color:#FEE2E2;color:#DC2626;border-radius:10px;"
                "font-size:10px;font-weight:bold;padding:1px 9px;";
    else
        style = "background-color:#FEF3C7;color:#D97706;border-radius:10px;"
                "font-size:10px;font-weight:bold;padding:1px 9px;";

    badge->setStyleSheet(style);
    return badge;
}

QLabel *EmployeesWidget::createRoleBadge(const QString &role)
{
    // Display Vietnamese label but use the English role string for logic checks
    QString displayRole;
    if      (role == "Manager") displayRole = "Quản lý";
    else if (role == "Admin")   displayRole = "Quản trị viên";
    else                        displayRole = "Nhân viên";

    QLabel *badge = new QLabel(displayRole);
    badge->setAlignment(Qt::AlignCenter);
    badge->setFixedHeight(20);

    QString style;
    if (role == "Manager")
        style = "background-color:#EDE9FE;color:#6D28D9;border-radius:10px;"
                "font-size:10px;font-weight:bold;padding:1px 9px;";
    else if (role == "Admin")
        style = "background-color:#DBEAFE;color:#1D4ED8;border-radius:10px;"
                "font-size:10px;font-weight:bold;padding:1px 9px;";
    else
        style = "background-color:#F1F5F9;color:#475569;border-radius:10px;"
                "font-size:10px;font-weight:bold;padding:1px 9px;";

    badge->setStyleSheet(style);
    return badge;
}

QPushButton *EmployeesWidget::createActionButton(const QString &text, const QString &tooltip)
{
    QPushButton *btn = new QPushButton();
    if (text.startsWith(":/images/"))
    {
        btn->setIcon(QIcon(text));
        btn->setIconSize(QSize(16, 16));
        btn->setFixedSize(28, 28);
    }
    else
    {
        btn->setText(text);
    }
    btn->setObjectName("tableActionBtn");
    btn->setToolTip(tooltip);
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}
