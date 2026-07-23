#include "global.h"
#include "employeeswidget.h"

// ============================================================
// Constructor / Destructor
// ============================================================

EmployeesWidget::EmployeesWidget(QWidget *parent) : QWidget(parent)
{
  setupUi();
  buildFilterDropdown();
  buildSortDropdown();
  setupConnections();
}

EmployeesWidget::~EmployeesWidget() {}

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

  profileLayout->addWidget(lblPageTitle);
  profileLayout->addStretch();

  mainLayout->addWidget(profileBlock);

  // --------------------------------------------------------
  // METRICS CARDS (3-column) — values updated in updateMetricCards()
  // --------------------------------------------------------
  metricsLayout = new QHBoxLayout();
  metricsLayout->setSpacing(14);

  // Payroll card — values filled dynamically
  m_payrollCard = createMetricCard(
      ":/images/dolar-svgrepo-com.svg", "#DBEAFE", "#2563EB",
      "Tổng bảng lương tháng", "-- vnđ",
      "↑ --% so với tháng trước", "+--% ", "#16A34A");
  // Staff card — values filled dynamically
  m_staffCard = createMetricCard(
      ":/images/people-svgrepo-com.svg", "#DCFCE7", "#16A34A",
      "Nhân viên đang làm việc", "0 / 0", "Hiện đang trong ca");
  // Absence card — values filled dynamically
  m_absenceCard = createMetricCard(
      ":/images/warning-circle-svgrepo-com.svg", "#FEF9C3", "#CA8A04",
      "Vắng mặt chờ duyệt", "0", "0 vắng · 0 chờ phê duyệt");

  metricsLayout->addWidget(m_payrollCard);
  metricsLayout->addWidget(m_staffCard);
  metricsLayout->addWidget(m_absenceCard);

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
  rosterSubtitle = new QLabel("Tổng cộng 0 nhân viên");
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
  employeesTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft |
                                                          Qt::AlignVCenter);
  employeesTable->horizontalHeader()->setHighlightSections(false);
  employeesTable->setFrameShape(QFrame::NoFrame);

  setupTableHeader();
  rosterLayout->addWidget(employeesTable);

  // Footer
  QFrame *footerFrame = new QFrame();
  footerFrame->setObjectName("tableFooterFrame");
  QHBoxLayout *footerLayout = new QHBoxLayout(footerFrame);
  footerLayout->setContentsMargins(18, 8, 18, 8);
  footerLabel = new QLabel(
      "Hiển thị 0 / 0 nhân viên  ·  0 đang làm, 0 vắng, 0 chờ duyệt");
  footerLabel->setObjectName("footerLabel");
  footerLayout->addWidget(footerLabel);
  footerLayout->addStretch();
  rosterLayout->addWidget(footerFrame);

  mainLayout->addWidget(rosterCard);
}

void EmployeesWidget::setupTableHeader()
{
  QStringList headers = {"MÃ NHÂN VIÊN", "TÊN", "VAI TRÒ", "LOẠI LƯƠNG",
                         "MỨC LƯƠNG", "TRẠNG THÁI", "THAO TÁC"};
  employeesTable->setHorizontalHeaderLabels(headers);

  QHeaderView *hdr = employeesTable->horizontalHeader();

  // Default: all columns resize to content
  hdr->setSectionResizeMode(QHeaderView::ResizeToContents);

  // Col 1 (Tên) stretches to fill remaining space
  hdr->setSectionResizeMode(1, QHeaderView::Stretch);

  // Col 2 (VAI TRÒ): fixed minimum wide enough for "Quản trị viên" badge
  hdr->setSectionResizeMode(2, QHeaderView::Fixed);
  hdr->resizeSection(2, 140);

  // Col 3 (LOẠI LƯƠNG): fixed minimum wide enough for "Theo giờ" badge
  hdr->setSectionResizeMode(3, QHeaderView::Fixed);
  hdr->resizeSection(3, 110);

  // Global minimum so no column collapses below readable size
  hdr->setMinimumSectionSize(72);
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

  chkStaff = new QCheckBox("Nhân viên");
  chkManager = new QCheckBox("Quản lý");
  chkAdmin = new QCheckBox("Quản trị viên");
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

  chkMale = new QCheckBox("Nam");
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
  layout->setSpacing(6);

  // Helper: build one sort option button
  auto makeSortBtn = [this, layout](const QString &label, const QString &field,
                                    int dir, const QString &iconPath)
  {
    QPushButton *btn = new QPushButton(label);
    btn->setObjectName("sortDropdownItem");
    btn->setFlat(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet("text-align: left; padding: 6px 12px;");

    layout->addWidget(btn);

    connect(btn, &QPushButton::clicked, this, [this, field, dir, iconPath]()
            {
      sortDropdown->hide();
      m_sortOpen = false;
      m_sortField = field;
      m_sortDir = dir;
      sortBtn->setIcon(QIcon(iconPath));
      emitUpdateRequest(); });
  };

  makeSortBtn("Mã nhân viên [\u2191 Tăng]", "id", 1,
              ":/images/sort-from-bottom-to-top.svg");
  makeSortBtn("Mã nhân viên [\u2193 Giảm]", "id", -1,
              ":/images/sort-from-top-to-bottom.svg");
  makeSortBtn("Tên [\u2191 Tăng]", "name", 1,
              ":/images/sort-from-bottom-to-top.svg");
  makeSortBtn("Tên [\u2193 Giảm]", "name", -1,
              ":/images/sort-from-top-to-bottom.svg");

  sortDropdown->adjustSize();
}

// ============================================================
// Connections
// ============================================================

void EmployeesWidget::setupConnections()
{
  // Add button
  connect(addEmployeeBtn, &QPushButton::clicked, this,
          &EmployeesWidget::handleAddEmployee);

  // Search bar — emit combined update when text changes
  connect(searchRoster, &QLineEdit::textChanged, this,
          &EmployeesWidget::emitUpdateRequest);

  // Filter dropdown toggle
  connect(filterBtn, &QPushButton::clicked, this,
          &EmployeesWidget::toggleFilterDropdown);

  // Filter checkboxes — emit combined update on any change
  connect(chkStaff, &QCheckBox::checkStateChanged, this,
          &EmployeesWidget::emitUpdateRequest);
  connect(chkManager, &QCheckBox::checkStateChanged, this,
          &EmployeesWidget::emitUpdateRequest);
  connect(chkAdmin, &QCheckBox::checkStateChanged, this,
          &EmployeesWidget::emitUpdateRequest);
  connect(chkMale, &QCheckBox::checkStateChanged, this,
          &EmployeesWidget::emitUpdateRequest);
  connect(chkFemale, &QCheckBox::checkStateChanged, this,
          &EmployeesWidget::emitUpdateRequest);

  // Sort dropdown toggle
  connect(sortBtn, &QPushButton::clicked, this,
          &EmployeesWidget::toggleSortDropdown);

  // (Event filter for profile block removed)
}

// ============================================================
// loadEmployees — called by Controller to update the table
// ============================================================

void EmployeesWidget::loadEmployees(const QList<User *> &employees)
{
  // The controller already applied filter→search→sort before calling us;
  // just render what we received.
  renderTable(employees);
  m_allEmployees = employees;
  updateMetricCards();
}

void EmployeesWidget::updateMetricCards()
{
  int total = m_allEmployees.size();
  // Since there is no status field in the model yet, all employees are
  // treated as "active" ("Đang làm") for now. Placeholders left for future.
  int active = total; // TODO: count by status when field added
  int absent = 0;     // TODO
  int pending = 0;    // TODO

  // ---- Staff card ----
  if (m_staffCard)
  {
    QLabel *val = m_staffCard->findChild<QLabel *>("metricValue");
    if (val)
      val->setText(QString("%1 / %2").arg(active).arg(total));
  }

  // ---- Absence card ----
  if (m_absenceCard)
  {
    QLabel *val = m_absenceCard->findChild<QLabel *>("metricValue");
    if (val)
      val->setText(QString::number(absent + pending));
    QLabel *sub = m_absenceCard->findChild<QLabel *>("metricSubtitle");
    if (sub)
      sub->setText(QString("%1 vắng · %2 chờ phê duyệt").arg(absent).arg(pending));
  }

  // ---- Payroll card (UI placeholder — logic not implemented yet) ----
  // Values left as placeholder; actual computation goes here in the future.
}

void EmployeesWidget::renderTable(const QList<User *> &employees)
{
  QStringList avatarColors = {"#3B82F6", "#10B981", "#F59E0B", "#EF4444",
                              "#8B5CF6", "#6366F1", "#14B8A6"};

  employeesTable->clearContents();
  employeesTable->setRowCount(employees.size());

  // ---- Dynamic subtitle & footer ----
  int total = m_allEmployees.size();
  int shown = employees.size();
  // Placeholder counts (no status field yet — treat all as active)
  int active = total;
  int absent = 0;
  int pending = 0;

  rosterSubtitle->setText(QString("Tổng cộng %1 nhân viên").arg(total));
  footerLabel->setText(
      QString("Hiển thị %1 / %2 nhân viên  ·  %3 đang làm, %4 vắng, %5 chờ duyệt")
          .arg(shown)
          .arg(total)
          .arg(active)
          .arg(absent)
          .arg(pending));

  for (int row = 0; row < employees.size(); ++row)
  {
    User *emp = employees[row];
    employeesTable->setRowHeight(row, 50);

    QString colorHex = avatarColors[row % avatarColors.size()];
    QString initials = emp->getName().left(2).toUpper();

    // Col 0 — ID
    QTableWidgetItem *idItem =
        new QTableWidgetItem(QString("NV-%1").arg(emp->getIdEmployee()));
    idItem->setForeground(QColor("#64748B"));
    idItem->setFont(QFont("Segoe UI", 9));
    idItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    idItem->setData(Qt::UserRole, emp->getRole());
    idItem->setData(Qt::UserRole + 1, emp->getIdEmployee());
    idItem->setData(Qt::UserRole + 2, emp->getGender());
    employeesTable->setItem(row, 0, idItem);

    // Col 1 — Name + avatar
    QWidget *nameWidget = new QWidget();
    QHBoxLayout *nameLayout = new QHBoxLayout(nameWidget);
    nameLayout->setContentsMargins(4, 4, 8, 4);
    nameLayout->setSpacing(10);
    nameLayout->addWidget(createAvatar(initials, colorHex));
    QLabel *nameLabel = new QLabel(emp->getName());
    nameLabel->setObjectName("empNameLabel");
    nameLabel->setFont(QFont("Segoe UI", 10, QFont::DemiBold));
    nameLayout->addWidget(nameLabel);
    nameLayout->addStretch();
    employeesTable->setCellWidget(row, 1, nameWidget);

    // Col 2 — Role badge: 0 left margin so badge aligns with header text
    QWidget *roleWidget = new QWidget();
    roleWidget->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *roleLayout = new QHBoxLayout(roleWidget);
    roleLayout->setContentsMargins(0, 4, 4, 4);
    roleLayout->setSpacing(0);
    QLabel *roleBadge = createRoleBadge(emp->getRole());
    roleBadge->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    roleLayout->addWidget(roleBadge, 0, Qt::AlignLeft | Qt::AlignVCenter);
    employeesTable->setCellWidget(row, 2, roleWidget);

    // Col 3 — Pay Type badge (replaces plain text)
    bool isHourly = (emp->getRole() == "Staff");
    QString payType = isHourly ? "Theo giờ" : "Cố định";
    QWidget *payWidget = new QWidget();
    payWidget->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *payLayout = new QHBoxLayout(payWidget);
    payLayout->setContentsMargins(0, 4, 4, 4);
    payLayout->setSpacing(0);
    QLabel *payBadge = createPayTypeBadge(payType);
    payBadge->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    payLayout->addWidget(payBadge, 0, Qt::AlignLeft | Qt::AlignVCenter);
    employeesTable->setCellWidget(row, 3, payWidget);

    // Col 4 — Salary in VNĐ  ("vnđ/h" for hourly, "vnđ/th" for monthly)
    // Calculation logic placeholder — space reserved for actual formula
    QString suffix = isHourly ? "vnđ/h" : "vnđ/th";
    QString rateStr = QString("%1 %2")
                          .arg(QString::number(emp->getSalary(), 'f', 0))
                          .arg(suffix);
    QTableWidgetItem *rateItem = new QTableWidgetItem(rateStr);
    rateItem->setForeground(QColor("#0F172A"));
    rateItem->setFont(QFont("Segoe UI", 9, QFont::DemiBold));
    rateItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    employeesTable->setItem(row, 4, rateItem);

    // Col 5 — Status badge: 0 left margin so badge aligns with header text
    QWidget *statusWidget = new QWidget();
    statusWidget->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *statusLayout = new QHBoxLayout(statusWidget);
    statusLayout->setContentsMargins(0, 4, 4, 4);
    statusLayout->setSpacing(0);
    statusLayout->addWidget(createStatusBadge("Đang làm"), 0,
                            Qt::AlignLeft | Qt::AlignVCenter);
    employeesTable->setCellWidget(row, 5, statusWidget);

    // Col 6 — Actions
    int empId = emp->getIdEmployee();
    QWidget *actionsWidget = new QWidget();
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setContentsMargins(6, 4, 6, 4);
    actionsLayout->setSpacing(5);

    QPushButton *editBtn =
        createActionButton(":/images/edit-svgrepo-com.svg", "Chỉnh sửa");
    QPushButton *delBtn =
        createActionButton(":/images/trash-bin-trash-svgrepo-com.svg", "Xóa");

    connect(editBtn, &QPushButton::clicked, this,
            [this, empId]()
            { emit requestEditEmployee(empId); });
    connect(delBtn, &QPushButton::clicked, this,
            [this, empId]()
            { emit requestDeleteEmployee(empId); });

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
  if (chkStaff->isChecked())
    contentFilter << "Staff";
  if (chkManager->isChecked())
    contentFilter << "Manage";
  if (chkAdmin->isChecked())
    contentFilter << "Admin";
  if (chkMale->isChecked())
    contentFilter << "Nam";
  if (chkFemale->isChecked())
    contentFilter << "Nữ";

  // change code, if have many field to sort
  QList<QString> contentSort;
  if (!m_sortField.isEmpty())
    contentSort.append(m_sortField);

  emit requestUpdate(searchText, contentFilter, contentSort, m_sortDir);
}

void EmployeesWidget::toggleFilterDropdown()
{
  if (m_filterOpen)
  {
    filterDropdown->hide();
    filterBtn->setIcon(QIcon(":/images/filter.svg"));
    m_filterOpen = false;
  }
  else
  {
    // Close sort dropdown first (only one open at a time)
    if (m_sortOpen)
    {
      sortDropdown->hide();
      m_sortOpen = false;
    }

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
  if (m_sortOpen)
  {
    sortDropdown->hide();
    m_sortOpen = false;
  }
  else
  {
    // Close filter dropdown first (only one open at a time)
    if (m_filterOpen)
    {
      filterDropdown->hide();
      filterBtn->setIcon(QIcon(":/images/filter.svg"));
      m_filterOpen = false;
    }

    QPoint pos = sortBtn->mapTo(this, QPoint(0, sortBtn->height() + 2));
    sortDropdown->adjustSize();
    sortDropdown->move(pos);
    sortDropdown->raise();
    sortDropdown->show();
    m_sortOpen = true;
  }
}

// ============================================================
// Slot — handleAddEmployee
// ============================================================

void EmployeesWidget::handleAddEmployee() { emit requestAddEmployee(); }

// ============================================================
// Widget Factories
// ============================================================

QLabel *EmployeesWidget::createAvatar(const QString &initials,
                                      const QString &bgColor)
{
  QLabel *avatar = new QLabel(initials);
  avatar->setObjectName("empAvatar");
  avatar->setFixedSize(32, 32);
  avatar->setAlignment(Qt::AlignCenter);
  avatar->setStyleSheet(QString("background-color: %1;"
                                "color: #FFFFFF;"
                                "border-radius: 16px;"
                                "font-size: 11px;"
                                "font-weight: bold;")
                            .arg(bgColor));
  return avatar;
}

QFrame *EmployeesWidget::createMetricCard(
    const QString &iconText, const QString &iconBg, const QString &iconColor,
    const QString &title, const QString &value, const QString &subtitle,
    const QString &badge, const QString &badgeColor)
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
    iconLabel->setPixmap(
        pix.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  }
  else
  {
    iconLabel->setText(iconText);
  }
  iconLabel->setObjectName("metricIcon");
  iconLabel->setFixedSize(50, 50);
  iconLabel->setAlignment(Qt::AlignCenter);
  iconLabel->setStyleSheet(QString("background-color: %1;"
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
        QString("color: %1; font-weight: bold; font-size: 10px;")
            .arg(badgeColor));
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
  badge->setFixedHeight(24);
  badge->setMinimumWidth(
      80); // Ensure enough width so padding doesn't clip text

  QString style;
  if (status == "Đang làm" || status == "Active")
    style = "background-color:#DCFCE7;color:#15803D;border-radius:12px;"
            "font-size:11px;font-weight:bold;padding:2px 10px;";
  else if (status == "Vắng" || status == "Absent")
    style = "background-color:#FEE2E2;color:#DC2626;border-radius:12px;"
            "font-size:11px;font-weight:bold;padding:2px 10px;";
  else
    style = "background-color:#FEF3C7;color:#D97706;border-radius:12px;"
            "font-size:11px;font-weight:bold;padding:2px 10px;";

  badge->setStyleSheet(style);
  return badge;
}

QLabel *EmployeesWidget::createRoleBadge(const QString &role)
{
  // Display Vietnamese label but use the English role string for logic checks
  QString displayRole;
  if (role == "Manage")
    displayRole = "Quản lý";
  else if (role == "Admin")
    displayRole = "Quản trị viên";
  else
    displayRole = "Nhân viên";

  QLabel *badge = new QLabel(displayRole);
  badge->setAlignment(Qt::AlignCenter);
  badge->setFixedHeight(24);
  // Width is computed from text — no hardcoded minimum needed
  // because the column itself is now fixed at 140px

  QString style;
  if (role == "Manage") // Fixed: was "Manager" — internal value is "Manage"
    style = "background-color:#EDE9FE;color:#6D28D9;border-radius:12px;"
            "font-size:11px;font-weight:bold;padding:2px 10px;";
  else if (role == "Admin")
    style = "background-color:#DBEAFE;color:#1D4ED8;border-radius:12px;"
            "font-size:11px;font-weight:bold;padding:2px 10px;";
  else // Staff — light blue instead of grey
    style = "background-color:#E0F2FE;color:#0369A1;border-radius:12px;"
            "font-size:11px;font-weight:bold;padding:2px 10px;";

  badge->setStyleSheet(style);
  // int textWidth = badge->fontMetrics().horizontalAdvance(displayRole);
  // badge->setFixedWidth(textWidth + 24);

  QFont font = badge->font();
  font.setBold(true);
  QFontMetrics fm(font);

  int textWidth = fm.horizontalAdvance(displayRole);
  badge->setFixedWidth(textWidth + 30);
  return badge;
}

QLabel *EmployeesWidget::createPayTypeBadge(const QString &payType)
{
  QLabel *badge = new QLabel(payType);
  badge->setAlignment(Qt::AlignCenter);
  badge->setFixedHeight(24);

  QString style;
  if (payType == "Theo giờ") // Hourly — warm orange
    style = "background-color:#FEF3C7;color:#B45309;border-radius:12px;"
            "font-size:11px;font-weight:bold;padding:2px 10px;";
  else // Cố định (Fixed) — indigo
    style = "background-color:#EEF2FF;color:#4338CA;border-radius:12px;"
            "font-size:11px;font-weight:bold;padding:2px 10px;";

  badge->setStyleSheet(style);
  int textWidth = badge->fontMetrics().horizontalAdvance(payType);
  badge->setFixedWidth(textWidth + 24);
  return badge;
}

QPushButton *EmployeesWidget::createActionButton(const QString &text,
                                                 const QString &tooltip)
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
