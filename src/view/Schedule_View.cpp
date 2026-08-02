#include "Schedule_View.h"
#include "global.h"
#include "ui_Schedule_View.h"
#include "utils/Config.h"
#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QFormLayout>
// ─── Shift label constants
// ────────────────────────────────────────────────────
static const QString SHIFT_NAMES[3] = {"Ca Sáng", "Ca Chiều", "Ca Tối"};
static const QString SHIFT_TIMES[3] = {"07:00 - 12:00", "12:00 - 17:00",
                                       "17:00 - 22:00"};

// Selected cell highlight colour (staff interactive grid)
static const QColor SELECTED_COLOR(0xBF, 0xDB, 0xFE);   // light blue
static const QColor SELECTED_BORDER(0x1D, 0x4E, 0xD8);  // blue-700

Schedule_View::Schedule_View(QWidget *parent)
    : QWidget(parent), ui(new Ui::Schedule_View), missingStaffWidget(nullptr),
      lblMissingStaffHeader(nullptr), lblMissingCount(nullptr),
      tableMissingStaff(nullptr)
{
  ui->setupUi(this);
  setUpUI();
  connect(ui->btnGenSchedule, &QPushButton::clicked, this,
          &Schedule_View::requestGenSchedule);
  connect(ui->btnConfirm, &QPushButton::clicked, this,
          &Schedule_View::requestConfirm);
  connect(ui->btnChangeShiftInfo, &QPushButton::clicked, this,
          &Schedule_View::onBtnChangeShiftInfoClicked);
  connect(ui->buttonLuu, &QPushButton::clicked, this,
          &Schedule_View::buttonSaveClicked);
}

Schedule_View::~Schedule_View() { delete ui; }

void Schedule_View::setUpUI()
{
  // ── Interactive grid (staff mode) ─────────────────────────────────────────
  ui->tableInteractiveGrid->setSelectionMode(QAbstractItemView::MultiSelection);
  ui->tableInteractiveGrid->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->tableInteractiveGrid->setFocusPolicy(Qt::StrongFocus);
  ui->tableInteractiveGrid->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->tableInteractiveGrid->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->tableInteractiveGrid->verticalHeader()->setMinimumWidth(110);
  ui->tableInteractiveGrid->setSelectionBehavior(QAbstractItemView::SelectItems);

  // ── Summary table (manager mode) ──────────────────────────────────────────
  ui->tableSum->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->tableSum->setFocusPolicy(Qt::NoFocus);
  ui->tableSum->setEditTriggers(QAbstractItemView::NoEditTriggers);

  // Force uppercase for horizontal headers of summary table
  for (int i = 0; i < ui->tableSum->columnCount(); ++i)
  {
    if (ui->tableSum->horizontalHeaderItem(i))
    {
      ui->tableSum->horizontalHeaderItem(i)->setText(
          ui->tableSum->horizontalHeaderItem(i)->text().toUpper());
    }
  }

  ui->tableSum->setSelectionMode(QAbstractItemView::NoSelection);

  this->setObjectName("ScheduleViewMain");
  this->setStyleSheet("QWidget { color: #1F2937; } QLabel { background-color: transparent; }");

  ui->buttonLuu->setStyleSheet(
      "QPushButton { background-color: #219653; color: white; border-radius: "
      "6px; padding: 8px 30px; font-weight: bold; font-size: 14px; } "
      "QPushButton:hover { background-color: #1E824C; }");

  ui->btnGenSchedule->setStyleSheet(
      "QPushButton { background-color: #A855F7; color: white; border-radius: "
      "6px; padding: 8px 20px; font-weight: bold; font-size: 14px; } "
      "QPushButton:hover { background-color: #9333EA; }");
  ui->btnGenSchedule->setVisible(false);

  ui->btnConfirm->setStyleSheet(
      "QPushButton { background-color: #219653; color: white; border-radius: "
      "6px; padding: 8px 20px; font-weight: bold; font-size: 14px; } "
      "QPushButton:hover { background-color: #1E824C; }");
  ui->btnConfirm->setVisible(false);

  ui->tableSum->setProperty("role", "staff");

  ui->frameTableContainer->setStyleSheet(
      "QFrame#frameTableContainer { "
      "background-color: #FFFFFF; "
      "border: 1px solid #E5E7EB; "
      "border-radius: 12px; "
      "}");

  ui->frameDangKyContainer->setStyleSheet(
      "QFrame#frameDangKyContainer { "
      "background-color: #FFFFFF; "
      "border: 1px solid #E5E7EB; "
      "border-radius: 12px; "
      "}");

  ui->tableSum->setStyleSheet(
      "QTableWidget { border: none; background-color: transparent; } "
      "QHeaderView::section { border-top: none; border-left: none; border-right: none; }");

  ui->tableInteractiveGrid->setStyleSheet(
      "QTableWidget { border: none; background-color: transparent; gridline-color: #E5E7EB; }"
      "QHeaderView::section { background-color: #1D4ED8; color: white; font-weight: bold; "
      "  padding: 6px; border: none; }"
      "QTableWidget::item { padding: 4px; border: 1px solid #E5E7EB; }"
      "QTableWidget::item:selected { background-color: #BFDBFE; color: #1D4ED8; border: 1px solid #1D4ED8; }");

  // ── Build missing staff widget (hidden until manager mode is set) ──────────
  missingStaffWidget = new QFrame(this);
  missingStaffWidget->setObjectName("frameMissingStaff");
  missingStaffWidget->setStyleSheet(
      "QFrame#frameMissingStaff { "
      "background-color: #FFFFFF; "
      "border: 1px solid #E5E7EB; "
      "border-radius: 12px; "
      "}");
  missingStaffWidget->setVisible(false);

  QVBoxLayout *missingLayout = new QVBoxLayout(missingStaffWidget);
  missingLayout->setContentsMargins(16, 16, 16, 16);

  // Header row: title + count pill
  QHBoxLayout *headerRow = new QHBoxLayout();
  lblMissingStaffHeader =
      new QLabel("CA LÀM THIẾU NHÂN VIÊN", missingStaffWidget);
  lblMissingStaffHeader->setStyleSheet("font-size: 14px; font-weight: bold; "
                                       "color: #1F2937; padding: 6px 10px; background-color: transparent;");

  lblMissingCount = new QLabel("0 ca cần xử lý", missingStaffWidget);
  lblMissingCount->setStyleSheet(
      "background-color: #FEE2E2; color: #991B1B; border-radius: 10px; "
      "padding: 3px 10px; font-size: 12px; font-weight: bold;");

  headerRow->addWidget(lblMissingStaffHeader);
  headerRow->addStretch();
  headerRow->addWidget(lblMissingCount);
  missingLayout->addLayout(headerRow);

  // Table
  tableMissingStaff = new QTableWidget(missingStaffWidget);
  tableMissingStaff->setColumnCount(5);
  tableMissingStaff->setHorizontalHeaderLabels(
      {"NGÀY", "CA LÀM", "YÊU CẦU", "ĐÃ XẾP", "THIẾU"});
  tableMissingStaff->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  tableMissingStaff->setSelectionMode(QAbstractItemView::NoSelection);
  tableMissingStaff->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tableMissingStaff->setFocusPolicy(Qt::NoFocus);
  tableMissingStaff->setAlternatingRowColors(true);
  tableMissingStaff->setStyleSheet(
      "QTableWidget { background-color: transparent; border: none; }"
      "QHeaderView::section { background-color: #2F80ED; color: white; "
      "font-weight: bold; padding: 7px; border: none; }"
      "QTableWidget::item { padding: 6px; color: #1F2937; }"
      "QTableWidget::item:alternate { background-color: #EFF6FF; }");
  missingLayout->addWidget(tableMissingStaff);

  // Add missing staff widget to the parent layout
  QLayout *parentLayout = ui->verticalLayout;
  if (parentLayout)
  {
    parentLayout->addWidget(missingStaffWidget);
  }
}

// ─── Build the interactive time-slot grid ────────────────────────────────────
void Schedule_View::buildInteractiveGrid(int openHour, int closeHour)
{
  QTableWidget *grid = ui->tableInteractiveGrid;

  int rowCount = closeHour - openHour; // e.g. 22-7 = 15
  grid->clearContents();
  grid->setRowCount(rowCount);
  grid->setColumnCount(7);

  // Row labels: "07:00 - 08:00", "08:00 - 09:00", …
  QStringList rowLabels;
  for (int h = openHour; h < closeHour; ++h)
  {
    rowLabels << QString("%1:00 - %2:00")
                     .arg(h, 2, 10, QChar('0'))
                     .arg(h + 1, 2, 10, QChar('0'));
  }
  grid->setVerticalHeaderLabels(rowLabels);
  grid->verticalHeader()->setVisible(true);

  // Column labels: Thứ 2 … Chủ Nhật (will be updated with dates by setUpInteractiveGrid)
  static const QStringList DAY_NAMES = {"Thứ 2", "Thứ 3", "Thứ 4", "Thứ 5",
                                        "Thứ 6", "Thứ 7", "Chủ Nhật"};
  grid->setHorizontalHeaderLabels(DAY_NAMES);

  // Populate cells (empty, selectable)
  for (int r = 0; r < rowCount; ++r)
  {
    grid->setRowHeight(r, 30);
    for (int c = 0; c < 7; ++c)
    {
      QTableWidgetItem *item = new QTableWidgetItem();
      item->setTextAlignment(Qt::AlignCenter);
      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      grid->setItem(r, c, item);
    }
  }
}

void Schedule_View::setUpInteractiveGrid(QDate monday, int openTime, int closeTime)
{
  m_openHour  = openTime;
  m_closeHour = closeTime;

  buildInteractiveGrid(openTime, closeTime);

  // Update column headers with actual dates
  QStringList headers;
  for (int i = 0; i < 7; ++i)
  {
    QDate d = monday.addDays(i);
    int dow = d.dayOfWeek(); // 1=Mon .. 7=Sun
    QString dayName = (dow == 7) ? "Chủ Nhật" : QString("Thứ %1").arg(dow + 1);
    headers << QString("%1\n%2").arg(dayName, d.toString("dd-MM-yyyy"));
  }
  ui->tableInteractiveGrid->setHorizontalHeaderLabels(headers);
}

void Schedule_View::updateStaffInteractiveGridStatus(
    const QMap<int, QList<Shift*>> &pendingShifts,
    const QMap<int, QList<Shift*>> &acceptedShifts,
    const QMap<int, QMap<int, ShiftBlock *>> &managerGrid)
{
    QTableWidget *grid = ui->tableInteractiveGrid;
    int rowCount = grid->rowCount();
    int openHour = m_openHour;

    for (int col = 0; col < 7; ++col) {
        for (int r = 0; r < rowCount; ++r) {
            int currentHour = openHour + r;
            QTableWidgetItem *item = grid->item(r, col);
            if (!item) continue;

            // Reset
            item->setText("");
            item->setBackground(Qt::white);
            item->setForeground(QColor(0x1F, 0x29, 0x37)); 
            QFont f = item->font();
            f.setBold(false);
            item->setFont(f);

            // 1. Check if the user has an accepted shift covering this hour
            bool isAccepted = false;
            if (acceptedShifts.contains(col)) {
                for (Shift* s : acceptedShifts[col]) {
                    if (s->getStartTime().hour() <= currentHour && s->getEndTime().hour() > currentHour) {
                        isAccepted = true;
                        break;
                    }
                }
            }
            if (isAccepted) {
                item->setText("Đã duyệt");
                item->setBackground(QColor("#D1FAE5")); // Green
                item->setForeground(QColor("#065F46"));
                f.setBold(true);
                item->setFont(f);
                continue;
            }

            // 2. Check if the user has a pending shift covering this hour
            bool isPending = false;
            if (pendingShifts.contains(col)) {
                for (Shift* s : pendingShifts[col]) {
                    if (s->getStartTime().hour() <= currentHour && s->getEndTime().hour() > currentHour) {
                        isPending = true;
                        break;
                    }
                }
            }
            if (isPending) {
                item->setText("Chờ duyệt");
                item->setBackground(QColor("#FEF9C3")); // Yellow
                item->setForeground(QColor("#854D0E"));
                f.setBold(true);
                item->setFont(f);
                continue;
            }

            // 3. If neither, check the manager grid to show global status
            int shiftRow = -1;
            if (currentHour >= 7 && currentHour < 12) shiftRow = 0;
            else if (currentHour >= 12 && currentHour < 17) shiftRow = 1;
            else if (currentHour >= 17 && currentHour < 22) shiftRow = 2;

            if (shiftRow >= 0 && managerGrid.contains(col) && managerGrid[col].contains(shiftRow)) {
                ShiftBlock *block = managerGrid[col][shiftRow];
                if (block) {
                    if (block->getStatus() == ShiftStatus::Empty || block->getStatus() == ShiftStatus::Understaffed) {
                        item->setText("Thiếu NV");
                        item->setBackground(QColor("#FEE2E2")); // Red
                        item->setForeground(QColor("#991B1B"));
                        f.setBold(true);
                        item->setFont(f);
                    } else {
                        item->setText("Đã đủ");
                        item->setBackground(QColor("#D1FAE5")); // Green
                        item->setForeground(QColor("#065F46"));
                        f.setBold(true);
                        item->setFont(f);
                    }
                }
            }
        }
    }
}

void Schedule_View::updateTableHeaders(QDate monday)
{
  QStringList headers;
  for (int i = 0; i < 7; ++i)
  {
    QDate d = monday.addDays(i);
    int dow = d.dayOfWeek();
    QString dayName = (dow == 7) ? "Chủ Nhật" : QString("Thứ %1").arg(dow + 1);
    headers << QString("%1\n%2").arg(dayName, d.toString("dd-MM-yyyy"));
  }
  ui->tableSum->setHorizontalHeaderLabels(headers);
}

void Schedule_View::enableRegistration(bool isEnable)
{
  ui->tableInteractiveGrid->setEnabled(isEnable);
  ui->buttonLuu->setEnabled(isEnable);

  if (isEnable)
  {
    ui->DangKyLich->setText("ĐĂNG KÝ LỊCH LÀM");
    ui->DangKyLich->setStyleSheet(
        "color: #333333; font-size: 16px; font-weight: bold;");
  }
  else
  {
    ui->DangKyLich->setText("CHƯA ĐẾN NGÀY ĐĂNG KÝ");
    ui->DangKyLich->setStyleSheet(
        "color: #E02424; font-size: 16px; font-weight: bold;");
  }
}

// updateStaffRegisteredGrid removed, handled by ViewSchedule_View


void Schedule_View::showError(const QString &mess)
{
  QMessageBox msgBox(this);
  msgBox.setWindowTitle("Lỗi");
  msgBox.setText(mess);
  msgBox.setIcon(QMessageBox::Warning);
  msgBox.setStyleSheet(
      "QMessageBox { background-color: #1e1e1e; } "
      "QLabel { color: #ffffff; background-color: transparent; } "
      "QPushButton { color: #ffffff; background-color: #333333; padding: 5px "
      "15px; border-radius: 3px; }");
  msgBox.exec();
}

// ─── Luu / Xac Nhan button ───────────────────────────────────────────────────
void Schedule_View::buttonSaveClicked()
{
  QTableWidget *grid = ui->tableInteractiveGrid;

  // Collect selected rows per column
  QList<QList<int>> selectedByDay(7);
  QSet<int> daysWithSelection;

  QList<QTableWidgetItem *> selected = grid->selectedItems();
  for (QTableWidgetItem *item : selected)
  {
    int col = item->column();
    int row = item->row();
    selectedByDay[col].append(row);
    daysWithSelection.insert(col);
  }

  // Validate minimum-days rule
  int minDays = Config::getMinDaysPerEmp();
  if (static_cast<int>(daysWithSelection.size()) < minDays)
  {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Chưa đủ số ngày đăng ký");
    msgBox.setText(
        QString("Bạn cần đăng ký tối thiểu %1 ngày trong tuần.\n"
                "Hiện tại bạn chỉ chọn %2 ngày.")
            .arg(minDays)
            .arg(daysWithSelection.size()));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStyleSheet(
        "QMessageBox { background-color: #1e1e1e; } "
        "QLabel { color: #ffffff; background-color: transparent; } "
        "QPushButton { color: #ffffff; background-color: #333333; padding: 5px "
        "15px; border-radius: 3px; }");
    msgBox.exec();
    return;
  }

  emit requestSaveGridShifts(selectedByDay);
}

void Schedule_View::showSuccess(const QString &msg)
{
  QMessageBox msgBox(this);
  msgBox.setWindowTitle("Xếp lịch thành công");
  msgBox.setText(msg);
  msgBox.setIcon(QMessageBox::Information);
  msgBox.setStyleSheet(
      "QMessageBox { background-color: #1e1e1e; } "
      "QLabel { color: #ffffff; background-color: transparent; } "
      "QPushButton { color: #ffffff; background-color: #333333; padding: 5px "
      "15px; border-radius: 3px; }");
  msgBox.exec();
}

void Schedule_View::showWarnings(const QStringList &warnings)
{
  QMessageBox dlg(this);
  dlg.setWindowTitle("Cảnh báo lịch làm việc");
  dlg.setIcon(QMessageBox::Warning);
  dlg.setText(
      QString("Xếp lịch hoàn tất nhưng có %1 cảnh báo:").arg(warnings.size()));
  dlg.setDetailedText(warnings.join("\n"));
  dlg.setStyleSheet("QMessageBox { background-color: #1e1e1e; } "
                    "QLabel { color: #ffffff; background-color: transparent; } "
                    "QPushButton { color: #ffffff; background-color: #333333; "
                    "padding: 5px 15px; border-radius: 3px; } "
                    "QTextEdit { background-color: #2b2b2b; color: #ffffff; "
                    "border: 1px solid #555; }");
  dlg.exec();
}
void Schedule_View::setManagerMode(bool isManager)
{
  int newMode = isManager ? 1 : 0;
  if (m_isAssignMode == newMode)
    return;
  m_isAssignMode = newMode;

  if (isManager)
  {
    ui->DangKyLich->setVisible(false);
    ui->tableInteractiveGrid->setVisible(false);
    ui->frameDangKyContainer->setVisible(false);
    ui->buttonLuu->setVisible(false);

    ui->btnGenSchedule->setVisible(true);
    ui->btnGenSchedule->setEnabled(true);
    ui->btnConfirm->setVisible(true);
    ui->btnConfirm->setEnabled(true);
    ui->XacNhanLich->setVisible(true);
    ui->frameTableContainer->setVisible(true);
    ui->XacNhanLich->setText("<html><head/><body><p><span style=\" "
                             "font-size:12pt; font-weight:700;\">TỔNG KẾT YÊU "
                             "CẦU ĐĂNG KÝ TRONG TUẦN</span></p></body></html>");

    // 3-shift manager grid
    ui->tableSum->setRowCount(3);
    ui->tableSum->setColumnCount(7);
    QStringList shiftLabels = {
        QString("%1\n%2").arg(SHIFT_NAMES[0], SHIFT_TIMES[0]),
        QString("%1\n%2").arg(SHIFT_NAMES[1], SHIFT_TIMES[1]),
        QString("%1\n%2").arg(SHIFT_NAMES[2], SHIFT_TIMES[2])};
    ui->tableSum->setVerticalHeaderLabels(shiftLabels);
    ui->tableSum->verticalHeader()->setVisible(true);
    ui->tableSum->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableSum->verticalHeader()->setMinimumWidth(130);
    ui->tableSum->verticalHeader()->setStyleSheet(
        "QHeaderView::section { background-color: #EFF6FF; color: #1D4ED8; "
        "font-weight: bold; "
        "border: 1px solid #BFDBFE; padding: 6px; text-transform: none; }");
    ui->tableSum->setSelectionMode(QAbstractItemView::NoSelection);

    ui->tableSum->setProperty("role", "manager");
    ui->tableSum->style()->unpolish(ui->tableSum);
    ui->tableSum->style()->polish(ui->tableSum);
    ui->tableSum->horizontalHeader()->style()->unpolish(
        ui->tableSum->horizontalHeader());
    ui->tableSum->horizontalHeader()->style()->polish(
        ui->tableSum->horizontalHeader());
    ui->tableSum->verticalHeader()->style()->unpolish(
        ui->tableSum->verticalHeader());
    ui->tableSum->verticalHeader()->style()->polish(
        ui->tableSum->verticalHeader());

    // Wire cell-click to emit shiftBlockClicked signal
    disconnect(ui->tableSum, &QTableWidget::cellClicked, this, nullptr);
    connect(ui->tableSum, &QTableWidget::cellClicked, this,
            [this](int row, int col)
            { emit shiftBlockClicked(col, row); });

    // Show missing staff widget
    if (missingStaffWidget)
      missingStaffWidget->setVisible(true);
  }
  else
  {
    ui->DangKyLich->setVisible(true);
    ui->tableInteractiveGrid->setVisible(true);
    ui->frameDangKyContainer->setVisible(true);
    ui->buttonLuu->setVisible(true);

    ui->btnGenSchedule->setVisible(false);
    ui->btnGenSchedule->setEnabled(false);
    ui->btnConfirm->setVisible(false);
    ui->btnConfirm->setEnabled(false);
    ui->XacNhanLich->setVisible(false);
    ui->frameTableContainer->setVisible(false);

    // Hide missing staff widget
    if (missingStaffWidget)
      missingStaffWidget->setVisible(false);
  }
}


// ─── Xem Lich Lam grid (accepted shifts) ─────────────────────────────────────
void Schedule_View::updateManagerPendingGrid(
    const QMap<int, QMap<int, ShiftBlock *>> &grid)
{
  for (int row = 0; row < 3; ++row)
    for (int col = 0; col < 7; ++col)
      ui->tableSum->removeCellWidget(row, col);
  ui->tableSum->clearContents();

  for (int col = 0; col < 7; ++col)
  {
    if (!grid.contains(col))
      continue;
    const QMap<int, ShiftBlock *> &dayData = grid[col];

    for (int row = 0; row < 3; ++row)
    {
      if (!dayData.contains(row))
        continue;
      ShiftBlock *block = dayData[row];
      if (!block)
        continue;

      int count = block->getStaffCount();

      QString cellBg, countStyle;
      if (count == 0)
      {
        cellBg     = "#FFF5F5";
        countStyle = "background-color:#FEE2E2;color:#991B1B";
      }
      else if (count < Config::getMinStaffPerShift())
      {
        cellBg     = "#FFFBEB";
        countStyle = "background-color:#FEF9C3;color:#854D0E";
      }
      else
      {
        cellBg     = "#F0FDF4";
        countStyle = "background-color:#D1FAE5;color:#065F46";
      }

      QString countText = QString("%1 nhân viên").arg(count);

      // Single Rich Text label instead of a full widget tree
      QString html = QString(
          "<div style='background-color:%1;border-radius:6px;padding:5px;'>"
          "<div style='color:#6B7280;font-size:9px;font-style:italic;'>Quan ly: ---</div>"
          "<div style='%2;border-radius:4px;padding:3px 6px;font-weight:bold;"
          "font-size:12px;text-align:center;margin-top:3px;'>%3</div>"
          "</div>")
          .arg(cellBg, countStyle, countText);

      QLabel *cellLabel = new QLabel(html);
      cellLabel->setTextFormat(Qt::RichText);
      cellLabel->setWordWrap(true);
      cellLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
      cellLabel->setContentsMargins(2, 2, 2, 2);

      ui->tableSum->setCellWidget(row, col, cellLabel);
    }
  }
}


// ─── Xep Lich Lam grid (all statuses — Pending/Accepted/Declined) ────────────
void Schedule_View::updateAssignGrid(
    const QMap<int, QMap<int, BlockCounts>> &counts)
{
  for (int row = 0; row < 3; ++row)
    for (int col = 0; col < 7; ++col)
      ui->tableSum->removeCellWidget(row, col);
  ui->tableSum->clearContents();

  for (int col = 0; col < 7; ++col)
  {
    if (!counts.contains(col))
      continue;
    for (int row = 0; row < 3; ++row)
    {
      if (!counts[col].contains(row))
        continue;
      BlockCounts bc = counts.value(col).value(row);

      int total = bc.pending + bc.accepted + bc.declined;

      // Determine total badge colour
      QString totalBadgeStyle;
      if (bc.pending > 0)
        totalBadgeStyle = "background-color:#DBEAFE;color:#1D4ED8";
      else if (total == 0)
        totalBadgeStyle = "background-color:#F3F4F6;color:#6B7280";
      else
        totalBadgeStyle = "background-color:#D1FAE5;color:#065F46";

      QString cellBg = (bc.pending > 0) ? "#EFF6FF" : "#F9FAFB";
      QString cellBorder = (bc.pending > 0) ? "#BFDBFE" : "#E5E7EB";

      // Build pills as inline HTML spans — no extra QLabel objects
      QString pills;
      pills += QString("<div style='background-color:#FEF9C3;color:#854D0E;"
                       "border-radius:4px;padding:4px 6px;font-size:11px;"
                       "font-weight:bold;margin-bottom:4px;'>%1 chờ</div>")
                   .arg(bc.pending);
      pills += QString("<div style='background-color:#D1FAE5;color:#065F46;"
                       "border-radius:4px;padding:4px 6px;font-size:11px;"
                       "font-weight:bold;margin-bottom:4px;'>%1 đã duyệt</div>")
                   .arg(bc.accepted);
      pills += QString("<div style='background-color:#FEE2E2;color:#991B1B;"
                       "border-radius:4px;padding:4px 6px;font-size:11px;"
                       "font-weight:bold;'>%1 từ chối</div>")
                   .arg(bc.declined);

      // Single Rich Text label instead of container widget + layout + multiple labels
      QString html = QString(
          "<div style='background-color:%1;border-radius:6px;"
          "border:1px solid %2;padding:5px;'>"
          "<div style='%3;border-radius:4px;padding:3px 6px;font-weight:bold;"
          "font-size:12px;text-align:center;margin-bottom:6px;'>%4 yêu cầu</div>"
          "<div style='text-align:center;'>%5</div>"
          "</div>")
          .arg(cellBg, cellBorder, totalBadgeStyle)
          .arg(total)
          .arg(pills);

      QLabel *cellLabel = new QLabel(html);
      cellLabel->setTextFormat(Qt::RichText);
      cellLabel->setWordWrap(true);
      cellLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
      cellLabel->setContentsMargins(2, 2, 2, 2);
      cellLabel->setCursor(Qt::PointingHandCursor);

      ui->tableSum->setCellWidget(row, col, cellLabel);
    }
  }
}


// ─── Shift Requests Popup Dialog ─────────────────────────────────────────────
void Schedule_View::showShiftRequestsDialog(
    const QList<PendingShiftInfo> &requests, const QString &shiftLabel)
{
  QDialog *dlg = new QDialog(this);
  dlg->setWindowTitle(QString("Yêu cầu — %1").arg(shiftLabel));
  dlg->setMinimumWidth(720);
  dlg->setMinimumHeight(400);
  dlg->setStyleSheet("QDialog { background-color: #F8FAFC; }");

  QVBoxLayout *mainLayout = new QVBoxLayout(dlg);
  mainLayout->setContentsMargins(18, 18, 18, 18);
  mainLayout->setSpacing(12);

  // Title
  QLabel *title = new QLabel(
      QString("DANH SÁCH YÊU CẦU — %1").arg(shiftLabel.toUpper()), dlg);
  title->setStyleSheet("font-size: 14px; font-weight: bold; color: #1F2937;");
  mainLayout->addWidget(title);

  // Table
  QTableWidget *tbl = new QTableWidget(requests.size(), 6, dlg);
  tbl->setHorizontalHeaderLabels(
      {"STT", "ID", "TÊN NHÂN VIÊN", "VAI TRÒ", "GIỜ LÀM", "HÀNH ĐỘNG"});
  tbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  tbl->horizontalHeader()->setSectionResizeMode(0,
                                                QHeaderView::ResizeToContents);
  tbl->horizontalHeader()->setSectionResizeMode(1,
                                                QHeaderView::ResizeToContents);
  tbl->horizontalHeader()->setSectionResizeMode(5,
                                                QHeaderView::ResizeToContents);
  tbl->setSelectionMode(QAbstractItemView::NoSelection);
  tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tbl->setFocusPolicy(Qt::NoFocus);
  tbl->setAlternatingRowColors(true);
  tbl->verticalHeader()->setVisible(false);
  tbl->setStyleSheet(
      "QTableWidget { background-color: #FFFFFF; border: 1px solid #E5E7EB; "
      "border-radius: 8px; }"
      "QHeaderView::section { background-color: #2F80ED; color: white; "
      "font-weight: bold; padding: 7px; border: none; }"
      "QTableWidget::item { padding: 6px; color: #1F2937; }"
      "QTableWidget::item:alternate { background-color: #F0F9FF; }");
  tbl->setRowHeight(0, 42);

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

  for (int i = 0; i < requests.size(); ++i)
  {
    const PendingShiftInfo &info = requests[i];

    tbl->setRowHeight(i, 42);
    tbl->setItem(i, 0, makeItem(QString::number(i + 1)));
    tbl->setItem(i, 1, makeItem(QString::number(info.employeeId)));
    tbl->setItem(i, 2, makeItem(info.employeeName));

    // Role badge
    QString roleDisplay = (info.role == "Staff") ? "Nhân viên" : "Quản lý";
    QString roleStyle =
        (info.role == "Staff")
            ? "background-color:#DBEAFE; color:#1D4ED8; border-radius:4px; "
              "padding:2px 8px; font-weight:bold; font-size:12px;"
            : "background-color:#EDE9FE; color:#5B21B6; border-radius:4px; "
              "padding:2px 8px; font-weight:bold; font-size:12px;";
    QLabel *roleLabel = new QLabel(roleDisplay, tbl);
    roleLabel->setStyleSheet(roleStyle);
    roleLabel->setAlignment(Qt::AlignCenter);
    QWidget *roleCell = new QWidget();
    QHBoxLayout *roleLayout = new QHBoxLayout(roleCell);
    roleLayout->setContentsMargins(4, 2, 4, 2);
    roleLayout->addStretch();
    roleLayout->addWidget(roleLabel);
    roleLayout->addStretch();
    tbl->setCellWidget(i, 3, roleCell);

    // Full working time (not cut to shift boundaries)
    QString timeStr = info.startTime.toString("HH:mm") + " - " +
                      info.endTime.toString("HH:mm");
    tbl->setItem(i, 4, makeItem(timeStr));

    // Actions column: depends on status
    QWidget *actionCell = new QWidget();
    QHBoxLayout *actionLayout = new QHBoxLayout(actionCell);
    actionLayout->setContentsMargins(4, 2, 4, 2);
    actionLayout->setSpacing(6);

    if (info.status == 0)
    {
      // Pending — show Approve + Decline
      QPushButton *btnApprove = new QPushButton("Cần duyện", actionCell);
      btnApprove->setStyleSheet(
          "QPushButton { background-color: #219653; color: white; "
          "border-radius: 4px; "
          "padding: 4px 10px; font-weight: bold; font-size: 11px; } "
          "QPushButton:hover { background-color: #1E824C; }");

      QPushButton *btnDecline = new QPushButton("Từ chối", actionCell);
      btnDecline->setStyleSheet(
          "QPushButton { background-color: #E02424; color: white; "
          "border-radius: 4px; "
          "padding: 4px 10px; font-weight: bold; font-size: 11px; } "
          "QPushButton:hover { background-color: #C81E1E; }");

      int shiftId = info.shiftId;
      connect(btnApprove, &QPushButton::clicked, this, [this, shiftId, dlg]()
              {
        emit requestApproveShift(shiftId);
        dlg->accept(); });
      connect(btnDecline, &QPushButton::clicked, this, [this, shiftId, dlg]()
              {
        emit requestDeclineShift(shiftId);
        dlg->accept(); });

      actionLayout->addWidget(btnApprove);
      actionLayout->addWidget(btnDecline);
    }
    else if (info.status == 1)
    {
      // Accepted
      QLabel *badge = new QLabel("Đã duyệt", actionCell);
      badge->setStyleSheet(
          "background-color:#D1FAE5; color:#065F46; border-radius:4px; "
          "padding:3px 8px; font-weight:bold; font-size:11px;");
      badge->setAlignment(Qt::AlignCenter);
      actionLayout->addStretch();
      actionLayout->addWidget(badge);
      actionLayout->addStretch();
    }
    else
    {
      // Declined — show at bottom (sorted already) with a "Tu choi" badge
      QLabel *badge = new QLabel("Từ chối", actionCell);
      badge->setStyleSheet(
          "background-color:#FEE2E2; color:#991B1B; border-radius:4px; "
          "padding:3px 8px; font-weight:bold; font-size:11px;");
      badge->setAlignment(Qt::AlignCenter);
      actionLayout->addStretch();
      actionLayout->addWidget(badge);
      actionLayout->addStretch();

      // Grey out the row slightly
      for (int c = 0; c < 5; ++c)
      {
        QTableWidgetItem *it = tbl->item(i, c);
        if (it)
          it->setForeground(QBrush(QColor(0xAA, 0xAA, 0xAA)));
      }
    }

    tbl->setCellWidget(i, 5, actionCell);
  }

  mainLayout->addWidget(tbl);

  // Close button
  QHBoxLayout *btnRow = new QHBoxLayout();
  btnRow->addStretch();
  QPushButton *btnClose = new QPushButton("Đóng", dlg);
  btnClose->setStyleSheet(
      "QPushButton { background-color: #6B7280; color: white; border-radius: "
      "6px; "
      "padding: 7px 24px; font-weight: bold; font-size: 13px; } "
      "QPushButton:hover { background-color: #4B5563; }");
  connect(btnClose, &QPushButton::clicked, dlg, &QDialog::reject);
  btnRow->addWidget(btnClose);
  mainLayout->addLayout(btnRow);

  dlg->exec();
  dlg->deleteLater();
}

void Schedule_View::updateManagerMissingShifts(
    const QList<MissingShiftInfo> &missingList)
{
  if (!tableMissingStaff)
    return;

  int missingCount = missingList.size();
  if (lblMissingCount)
  {
    lblMissingCount->setText(QString("%1 ca cần xử lý").arg(missingCount));
    lblMissingCount->setStyleSheet(
        missingCount > 0
            ? "background-color:#FEE2E2; color:#991B1B; border-radius:10px; "
              "padding:3px 10px; font-size:12px; font-weight:bold;"
            : "background-color:#D1FAE5; color:#065F46; border-radius:10px; "
              "padding:3px 10px; font-size:12px; font-weight:bold;");
  }

  tableMissingStaff->setRowCount(missingList.size());
  tableMissingStaff->clearContents();

  for (int i = 0; i < missingList.size(); ++i)
  {
    const MissingShiftInfo &info = missingList[i];
    int deficit = info.assigned - info.required; // negative number

    auto makeItem = [](const QString &text) -> QTableWidgetItem *
    {
      QTableWidgetItem *item = new QTableWidgetItem(text);
      item->setTextAlignment(Qt::AlignCenter);
      item->setForeground(QBrush(QColor(0x1F, 0x29, 0x37)));
      return item;
    };

    // Date
    tableMissingStaff->setItem(i, 0, makeItem(info.dateStr));

    // Shift name badge
    QLabel *shiftBadge = new QLabel(info.shiftName);
    shiftBadge->setAlignment(Qt::AlignCenter);
    shiftBadge->setStyleSheet(
        "background-color: transparent; color: #1D4ED8; "
        "padding: 2px 8px; font-weight: bold; font-size: 12px;");
    QWidget *badgeContainer = new QWidget();
    QHBoxLayout *badgeLayout = new QHBoxLayout(badgeContainer);
    badgeLayout->setContentsMargins(6, 2, 6, 2);
    badgeLayout->addStretch();
    badgeLayout->addWidget(shiftBadge);
    badgeLayout->addStretch();
    tableMissingStaff->setCellWidget(i, 1, badgeContainer);

    // Required
    tableMissingStaff->setItem(i, 2, makeItem(QString::number(info.required)));

    // Assigned
    tableMissingStaff->setItem(i, 3, makeItem(QString::number(info.assigned)));

    // Deficit (red)
    QTableWidgetItem *deficitItem =
        new QTableWidgetItem(QString::number(deficit));
    deficitItem->setTextAlignment(Qt::AlignCenter);
    deficitItem->setForeground(QBrush(QColor(0xDC, 0x26, 0x26)));
    QFont f = deficitItem->font();
    f.setBold(true);
    deficitItem->setFont(f);
    tableMissingStaff->setItem(i, 4, deficitItem);
  }
}

void Schedule_View::onBtnChangeShiftInfoClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Thay Đổi Thông Tin Ca Làm");
    dialog.setMinimumWidth(350);

    QFormLayout *layout = new QFormLayout(&dialog);

    QComboBox *cbDay = new QComboBox();
    cbDay->addItems({"Thứ 2", "Thứ 3", "Thứ 4", "Thứ 5", "Thứ 6", "Thứ 7", "Chủ Nhật"});
    layout->addRow("Chọn ngày:", cbDay);

    QComboBox *cbShift = new QComboBox();
    cbShift->addItems({"Ca Sáng", "Ca Chiều", "Ca Tối"});
    layout->addRow("Chọn ca làm:", cbShift);

    QSpinBox *sbNum = new QSpinBox();
    sbNum->setRange(1, 20);
    layout->addRow("Số lượng nhân viên:", sbNum);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        // TODO: Pass the chosen values to the controller to update Config or Schedule_Model
        // For example:
        // emit requestChangeShiftInfo(cbDay->currentIndex(), cbShift->currentIndex(), sbNum->value());
        qDebug() << "TODO: Update shift requirement for" 
                 << cbDay->currentText() << cbShift->currentText() 
                 << "to" << sbNum->value() << "employees";
    }
}
