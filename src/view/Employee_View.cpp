#include "global.h"
#include "Employee_View.h"
#include "ui_Employee_View.h"
#include "AddEmployee_Dialog.h"
#include "EditEmployee_Dialog.h"
#include "EmployeeDetails_Dialog.h"
#include "utils/SessionManage.h"
#include <QTimer>
#include <QMenu>

// ============================================================
// Constructor / Destructor
// ============================================================

Employee_View::Employee_View(QWidget *parent) : QWidget(parent), ui(new Ui::Employee_View)
{
  ui->setupUi(this);

  // Set icons on buttons that cannot be specified inside the .ui file
  // (icon paths reference Qt resources compiled at build time)
  ui->filterBtn->setIcon(QIcon(":/images/filter.svg"));
  ui->sortBtn->setIcon(QIcon(":/images/sort-vertical-svgrepo-com.svg"));

  // Metric cards removed


  setupTableHeader();
  buildFilterDropdown();
  buildSortDropdown();
  setupConnections();
}

Employee_View::~Employee_View()
{
  delete ui;
}

// ============================================================
// Table Header Setup
// ============================================================

void Employee_View::setupTableHeader()
{
  // Insert column 5 for hours worked
  ui->employeesTable->insertColumn(5);
  QTableWidgetItem* hoursHeader = new QTableWidgetItem("GIỜ LÀM (THÁNG NÀY)");
  ui->employeesTable->setHorizontalHeaderItem(5, hoursHeader);

  QHeaderView *hdr = ui->employeesTable->horizontalHeader();

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

  hdr->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  hdr->setHighlightSections(false);
}

// ============================================================
// Build Filter Dropdown (floating child widget)
// ============================================================

void Employee_View::buildFilterDropdown()
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

  chkCashier = new QCheckBox("Thu ngân");
  chkHallStaff = new QCheckBox("Nhân viên sảnh");
  chkKitchenAssistant = new QCheckBox("Phụ bếp");
  chkManager = new QCheckBox("Quản lý");
  chkAdmin = new QCheckBox("Quản trị viên");
  layout->addWidget(chkCashier);
  layout->addWidget(chkHallStaff);
  layout->addWidget(chkKitchenAssistant);
  layout->addWidget(chkManager);
  //layout->addWidget(chkAdmin);

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

void Employee_View::buildSortDropdown()
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
      ui->sortBtn->setIcon(QIcon(iconPath));
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

void Employee_View::setupConnections()
{
  // Add button
  connect(ui->addEmployeeBtn, &QPushButton::clicked, this,
          &Employee_View::handleAddEmployee);

  // Search bar — debounce 30ms to avoid rebuilding table on every keystroke (Fix 1)
  m_searchTimer = new QTimer(this);
  m_searchTimer->setSingleShot(true);
  m_searchTimer->setInterval(150);
  connect(m_searchTimer, &QTimer::timeout, this, &Employee_View::emitUpdateRequest);
  connect(ui->searchRoster, &QLineEdit::textChanged, this, [this]() {
      m_searchTimer->start();
  });

  // Filter dropdown toggle
  connect(ui->filterBtn, &QPushButton::clicked, this,
          &Employee_View::toggleFilterDropdown);

  // Filter checkboxes — emit combined update on any change
  connect(chkCashier, &QCheckBox::checkStateChanged, this,
          &Employee_View::emitUpdateRequest);
  connect(chkHallStaff, &QCheckBox::checkStateChanged, this,
          &Employee_View::emitUpdateRequest);
  connect(chkKitchenAssistant, &QCheckBox::checkStateChanged, this,
          &Employee_View::emitUpdateRequest);
  connect(chkManager, &QCheckBox::checkStateChanged, this,
          &Employee_View::emitUpdateRequest);
  connect(chkAdmin, &QCheckBox::checkStateChanged, this,
          &Employee_View::emitUpdateRequest);
  connect(chkMale, &QCheckBox::checkStateChanged, this,
          &Employee_View::emitUpdateRequest);
  connect(chkFemale, &QCheckBox::checkStateChanged, this,
          &Employee_View::emitUpdateRequest);

  // Sort dropdown toggle
  connect(ui->sortBtn, &QPushButton::clicked, this,
          &Employee_View::toggleSortDropdown);
          
  // Double click table to view details
  connect(ui->employeesTable, &QTableWidget::cellDoubleClicked, this, [this](int row, int col) {
      if (row < 0 || row >= m_allEmployees.size()) return;
      User* emp = m_allEmployees[row];
      EmployeeDetails_Dialog dlg(emp, this);
      dlg.exec();
  });
}

// ============================================================
// loadEmployees — called by Controller to update the table
// ============================================================

void Employee_View::loadEmployees(const QList<User *> &employees, const QMap<int, double> &hoursMap)
{
  // The controller already applied filter→search→sort before calling us;
  // just render what we received.
  m_allEmployees = employees;
  ui->employeesTable->clearContents();
  ui->employeesTable->setRowCount(employees.size());

  // ---- Dynamic subtitle & footer ----
  int total = m_allEmployees.size();
  int shown = employees.size();
  
  int cashierCount = 0;
  int hallStaffCount = 0;
  int kitchenCount = 0;
  
  for (User* emp : employees) {
      if (emp->getRole() == "Cashier") cashierCount++;
      else if (emp->getRole() == "HallStaff") hallStaffCount++;
      else if (emp->getRole() == "KitchenAssistant") kitchenCount++;
  }
  
  QString breakdown = QString(" (gồm %1 Thu ngân, %2 Nhân viên sảnh, %3 Phụ bếp)")
                          .arg(cashierCount)
                          .arg(hallStaffCount)
                          .arg(kitchenCount);
                          
  ui->rosterSubtitle->setText(QString("Tổng cộng %1 nhân viên").arg(total));
  ui->footerLabel->setText(
      QString("Hiển thị %1 / %2 nhân viên%3")
          .arg(shown)
          .arg(total)
          .arg(breakdown));


  ui->employeesTable->setUpdatesEnabled(false);

  for (int row = 0; row < employees.size(); ++row)
  {
    User *emp = employees[row];
    ui->employeesTable->setRowHeight(row, 50);

    QString avatarPath = emp->getAvatarPath();

    // Col 0 — ID
    QTableWidgetItem *idItem =
        new QTableWidgetItem(QString("NV-%1").arg(emp->getIdEmployee()));
    idItem->setForeground(QColor(0x64748B));
    idItem->setFont(QFont("Segoe UI", 9));
    idItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    idItem->setData(Qt::UserRole, emp->getRole());
    idItem->setData(Qt::UserRole + 1, emp->getIdEmployee());
    idItem->setData(Qt::UserRole + 2, emp->getGender());
    ui->employeesTable->setItem(row, 0, idItem);

    // Col 1 — Name + avatar
    QWidget *nameWidget = new QWidget();
    QHBoxLayout *nameLayout = new QHBoxLayout(nameWidget);
    nameLayout->setContentsMargins(4, 4, 8, 4);
    nameLayout->setSpacing(10);
    nameLayout->addWidget(createAvatar(avatarPath));
    QLabel *nameLabel = new QLabel(emp->getName());
    nameLabel->setObjectName("empNameLabel");
    nameLabel->setFont(QFont("Segoe UI", 10, QFont::DemiBold));
    nameLabel->setStyleSheet("color: black;");
    nameLayout->addWidget(nameLabel);
    nameLayout->addStretch();
    ui->employeesTable->setCellWidget(row, 1, nameWidget);

    // Col 2 — Role badge: 0 left margin so badge aligns with header text
    QWidget *roleWidget = new QWidget();
    roleWidget->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *roleLayout = new QHBoxLayout(roleWidget);
    roleLayout->setContentsMargins(0, 4, 4, 4);
    roleLayout->setSpacing(0);
    QLabel *roleBadge = createRoleBadge(emp->getRole());
    roleBadge->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    roleLayout->addWidget(roleBadge, 0, Qt::AlignLeft | Qt::AlignVCenter);
    ui->employeesTable->setCellWidget(row, 2, roleWidget);

    // Col 3 — Pay Type badge (replaces plain text)
    bool isHourly = !emp->getIsFixedSalary();
    QString payType = isHourly ? "Theo giờ" : "Cố định";
    QWidget *payWidget = new QWidget();
    payWidget->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *payLayout = new QHBoxLayout(payWidget);
    payLayout->setContentsMargins(0, 4, 4, 4);
    payLayout->setSpacing(0);
    QLabel *payBadge = createPayTypeBadge(payType);
    payBadge->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    payLayout->addWidget(payBadge, 0, Qt::AlignLeft | Qt::AlignVCenter);
    ui->employeesTable->setCellWidget(row, 3, payWidget);

    // Col 4 — Salary in VNĐ  ("vnđ/h" for hourly, "vnđ/th" for monthly)
    QString suffix = isHourly ? "vnđ/h" : "vnđ/th";
    QString rateStr = QString("%1 %2")
                          .arg(QString::number(emp->getBaseSalary(), 'f', 0))
                          .arg(suffix);
    QTableWidgetItem *rateItem = new QTableWidgetItem(rateStr);
    rateItem->setForeground(QColor(0x0F172A));
    rateItem->setFont(QFont("Segoe UI", 9, QFont::DemiBold));
    rateItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->employeesTable->setItem(row, 4, rateItem);

    // Col 5 — Hours worked this month
    double hours = hoursMap.value(emp->getIdEmployee(), 0.0);
    QTableWidgetItem *hoursItem = new QTableWidgetItem(QString("%1 giờ").arg(hours, 0, 'f', 1));
    hoursItem->setForeground(QColor(0x334155));
    hoursItem->setFont(QFont("Segoe UI", 9));
    hoursItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->employeesTable->setItem(row, 5, hoursItem);

    // Col 6 — Phone number
    QTableWidgetItem *phoneItem = new QTableWidgetItem(emp->getPhoneNum());
    phoneItem->setForeground(QColor(0x334155));
    phoneItem->setFont(QFont("Segoe UI", 9));
    phoneItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->employeesTable->setItem(row, 6, phoneItem);

    // Col 7 — Status badge
    QString statusText;
    QString badgeStyle;
    if (emp->getStatus() == "suspended") {
        statusText = "Hoãn làm";
        badgeStyle = "background-color:#FEF3C7;color:#D97706;border-radius:12px;"
                     "font-size:11px;font-weight:bold;padding:2px 10px;";
    } else {
        statusText = "Đang làm";
        badgeStyle = "background-color:#DCFCE7;color:#15803D;border-radius:12px;"
                     "font-size:11px;font-weight:bold;padding:2px 10px;";
    }
    QWidget *statusWidget = new QWidget();
    statusWidget->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *statusLayout = new QHBoxLayout(statusWidget);
    statusLayout->setContentsMargins(0, 4, 4, 4);
    statusLayout->setSpacing(0);
    QLabel *statusBadge = new QLabel(statusText);
    statusBadge->setAlignment(Qt::AlignCenter);
    statusBadge->setFixedHeight(24);
    statusBadge->setMinimumWidth(80);
    statusBadge->setStyleSheet(badgeStyle);
    statusLayout->addWidget(statusBadge, 0, Qt::AlignLeft | Qt::AlignVCenter);
    ui->employeesTable->setCellWidget(row, 7, statusWidget);

    // Col 8 — Actions
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

    bool isOtherManager = false;
    User* currentUser = SessionManager::getInstance()->getCurrentUser();
    if (currentUser) {
        if ((emp->getRole() == "Manager" || emp->getRole() == "Admin") &&
            emp->getIdEmployee() != currentUser->getIdEmployee()) {
            isOtherManager = true;
        }
    }

    if (!isOtherManager) {
        actionsLayout->addWidget(editBtn);
        actionsLayout->addWidget(delBtn);
    } else {
        editBtn->setVisible(false);
        delBtn->setVisible(false);
    }
    actionsLayout->addStretch();
    ui->employeesTable->setCellWidget(row, 8, actionsWidget);
  }

  ui->employeesTable->setUpdatesEnabled(true);
}


// ============================================================
// showError / showSuccess
// ============================================================

void Employee_View::showError(const QString &msg)
{
  QMessageBox::critical(this, "Lỗi", msg);
}

void Employee_View::showSuccess(const QString &msg)
{
  QMessageBox::information(this, "Thành công", msg);
}

// ============================================================
// emitUpdateRequest — collects all active criteria and signals the Controller
// ============================================================

void Employee_View::emitUpdateRequest()
{
  QString searchText = ui->searchRoster->text();

  QList<QString> contentFilter;
  if (chkCashier->isChecked())
    contentFilter << "Cashier";
  if (chkHallStaff->isChecked())
    contentFilter << "HallStaff";
  if (chkKitchenAssistant->isChecked())
    contentFilter << "KitchenAssistant";
  if (chkManager->isChecked())
    contentFilter << "Manager";
  if (chkAdmin->isChecked())
    contentFilter << "Admin";
  if (chkMale->isChecked())
    contentFilter << "Nam";
  if (chkFemale->isChecked())
    contentFilter << "Nữ";

  QList<QString> contentSort;
  if (!m_sortField.isEmpty())
    contentSort.append(m_sortField);

  emit requestUpdate(searchText, contentFilter, contentSort, m_sortDir);
}

// ============================================================
// Filter / Sort Dropdown Toggles
// ============================================================

void Employee_View::toggleFilterDropdown()
{
  if (m_filterOpen)
  {
    filterDropdown->hide();
    ui->filterBtn->setIcon(QIcon(":/images/filter.svg"));
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
    QPoint pos = ui->filterBtn->mapTo(this, QPoint(0, ui->filterBtn->height() + 2));
    filterDropdown->adjustSize();
    filterDropdown->move(pos);
    filterDropdown->raise();
    filterDropdown->show();
    ui->filterBtn->setIcon(QIcon(":/images/filter-slash.svg"));
    m_filterOpen = true;
  }
}

void Employee_View::toggleSortDropdown()
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
      ui->filterBtn->setIcon(QIcon(":/images/filter.svg"));
      m_filterOpen = false;
    }

    QPoint pos = ui->sortBtn->mapTo(this, QPoint(0, ui->sortBtn->height() + 2));
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

void Employee_View::handleAddEmployee() { emit requestAddEmployee(); }

// ============================================================
// Widget Factories
// ============================================================

QLabel *Employee_View::createAvatar(const QString &avatarPath)
{
  const int size = 32;

  // Fix 3: Check cache first — skip disk I/O and QPainter if already rendered
  const QString cacheKey = avatarPath.isEmpty() ? QStringLiteral("__default__") : avatarPath;
  if (!m_avatarCache.contains(cacheKey)) {
      QPixmap avatarPixmap;
      if (!avatarPath.isEmpty()) {
          if (avatarPath.startsWith(":/")) {
              avatarPixmap.load(avatarPath);
          } else {
              QDir appDir(QCoreApplication::applicationDirPath());
              appDir.cdUp();
              appDir.cdUp();
              QString fullPath = appDir.filePath("resources/avatars/") + avatarPath;
              if (QFile::exists(fullPath))
                  avatarPixmap.load(fullPath);
          }
      }
      if (avatarPixmap.isNull())
          avatarPixmap.load(":/images/avatarSample.png");

      QPixmap scaled = avatarPixmap.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
      QPixmap rounded(size, size);
      rounded.fill(Qt::transparent);
      QPainter painter(&rounded);
      painter.setRenderHint(QPainter::Antialiasing, true);
      painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
      QPainterPath path;
      path.addRoundedRect(0, 0, size, size, size / 2, size / 2);
      painter.setClipPath(path);
      int xOffset = (size - scaled.width()) / 2;
      int yOffset = (size - scaled.height()) / 2;
      painter.drawPixmap(xOffset, yOffset, scaled);
      painter.end();

      m_avatarCache.insert(cacheKey, rounded);
  }

  QLabel *avatar = new QLabel();
  avatar->setObjectName("empAvatar");
  avatar->setFixedSize(size, size);
  avatar->setAlignment(Qt::AlignCenter);
  avatar->setPixmap(m_avatarCache[cacheKey]);
  return avatar;
}




QLabel *Employee_View::createRoleBadge(const QString &role)
{
  // Display Vietnamese label but use the English role string for logic checks
    QString displayRole;
    if (role == "Manager")
        displayRole = "Quản lý";
    else if (role == "Admin")
        displayRole = "Quản trị viên";
    else if (role == "Cashier")
        displayRole = "Thu ngân";
    else if (role == "HallStaff")
        displayRole = "Nhân viên sảnh";
    else if (role == "KitchenAssistant")
        displayRole = "Phụ bếp";
    else displayRole = "Nhân viên";

  QLabel *badge = new QLabel(displayRole);
  badge->setAlignment(Qt::AlignCenter);
  badge->setFixedHeight(24);

  QString style;
  if (role == "Manager")
      style = "background-color:#EDE9FE;color:#6D28D9;border-radius:12px;" // Pastel Tím
              "font-size:11px;font-weight:bold;padding:2px 10px;";
  else if (role == "Admin")
      style = "background-color:#DBEAFE;color:#1D4ED8;border-radius:12px;" // Pastel Xanh biển đậm
              "font-size:11px;font-weight:bold;padding:2px 10px;";
  else if (role == "Cashier")
      style = "background-color:#CCFBF1;color:#0F766E;border-radius:12px;" // Pastel Xanh ngọc (Teal)
              "font-size:11px;font-weight:bold;padding:2px 10px;";
  else if (role == "HallStaff")
      style = "background-color:#FFEDD5;color:#C2410C;border-radius:12px;" // Pastel Cam (Orange)
              "font-size:11px;font-weight:bold;padding:2px 10px;";
  else if (role == "KitchenAssistant")
      style = "background-color:#FFE4E6;color:#BE123C;border-radius:12px;" // Pastel Đỏ hồng (Rose)
              "font-size:11px;font-weight:bold;padding:2px 10px;";
  else // Staff mặc định
      style = "background-color:#E0F2FE;color:#0369A1;border-radius:12px;" // Pastel Xanh da trời nhạt
              "font-size:11px;font-weight:bold;padding:2px 10px;";


  badge->setStyleSheet(style);

  QFont font = badge->font();
  font.setBold(true);
  QFontMetrics fm(font);

  int textWidth = fm.horizontalAdvance(displayRole);
  badge->setFixedWidth(textWidth + 30);
  return badge;
}

QLabel *Employee_View::createPayTypeBadge(const QString &payType)
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

QPushButton *Employee_View::createActionButton(const QString &text,
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
