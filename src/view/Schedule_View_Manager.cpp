#include "Schedule_View.h"
#include "global.h"
#include "ui_Schedule_View.h"
#include <QProgressBar>
#include <QScrollArea>

namespace {
static const QString SHIFT_NAMES[3] = {"Ca Sáng", "Ca Chiều", "Ca Tối"};
static const QString SHIFT_TIMES[3] = {"07:00 - 12:00", "12:00 - 17:00",
                                       "17:00 - 22:00"};

void clearLayoutItems(QLayout *layout)
{
  if (!layout) return;
  while (QLayoutItem *item = layout->takeAt(0))
  {
    if (QLayout *childLayout = item->layout())
    {
      clearLayoutItems(childLayout);
      delete childLayout;
      continue;
    }
    if (QWidget *widget = item->widget())
      widget->deleteLater();
    delete item;
  }
}
} // namespace

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

void Schedule_View::setManagerMode(bool isManager)
{
  int newMode = isManager ? 1 : 0;
  if (m_isAssignMode == newMode)
    return;
  m_isAssignMode = newMode;

  if (isManager)
  {
    if (QWidget *toolbar = findChild<QWidget *>("managerToolbar"))
      toolbar->setVisible(true);
    if (QWidget *summaryBar = findChild<QWidget *>("managerSummaryBar"))
      summaryBar->setVisible(true);
    if (fullTimeInfoWidget)
      fullTimeInfoWidget->setVisible(false);
    if (lblFullTimeFooterMessage)
      lblFullTimeFooterMessage->setVisible(false);
    if (partTimeInfoWidget)
      partTimeInfoWidget->setVisible(false);
    if (lblPartTimeFooterMessage)
      lblPartTimeFooterMessage->setVisible(false);
    ui->DangKyLich->setVisible(false);
    ui->tableInteractiveGrid->setVisible(false);
    ui->frameDangKyContainer->setVisible(false);
    ui->buttonLuu->setVisible(false);

    ui->btnGenSchedule->setVisible(true);
    ui->btnGenSchedule->setEnabled(true);
    ui->btnConfirm->setVisible(true);
    ui->btnConfirm->setEnabled(true);
    ui->btnConfirm->setText("Xem lại & công bố");
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
            {
              selectManagerShift(col, row);
              emit shiftBlockClicked(col, row);
            });

    // Show missing staff widget
    if (missingStaffWidget)
      missingStaffWidget->setVisible(true);
  }
  else
  {
    if (QWidget *toolbar = findChild<QWidget *>("managerToolbar"))
      toolbar->setVisible(false);
    if (QWidget *summaryBar = findChild<QWidget *>("managerSummaryBar"))
      summaryBar->setVisible(false);
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
      else if (count < 0) //Config::getMinStaffForRole(this->controller))
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
#if 0
      pills = QString("<div style='color:#374151;font-size:12px;font-weight:bold;'>Cần %1 · Đã xếp %2 · Thiếu %3</div>")
                  .arg(bc.required).arg(bc.accepted).arg(shortage);
      if (bc.pending > 0)
        pills += QString("<div style='color:#92400E;font-size:11px;margin-top:4px;'>%1 yêu cầu chờ xử lý</div>").arg(bc.pending);
      if (bc.declined > 0)
        pills += QString("<div style='color:#991B1B;font-size:10px;margin-top:2px;'>%1 từ chối</div>").arg(bc.declined);

#endif
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
void Schedule_View::selectManagerShift(int dayColumn, int shiftRow)
{
  if (dayColumn < 0 || dayColumn >= 7 || shiftRow < 0 || shiftRow >= 3)
    return;
  m_selectedManagerDay = dayColumn;
  m_selectedManagerShift = shiftRow;
  if (!m_lastAssignCounts.isEmpty())
    updateAssignGrid(m_lastAssignCounts);
}

void Schedule_View::updateAssignGrid(
    const QMap<int, QMap<int, BlockCounts>> &counts)
{
  m_lastAssignCounts = counts;
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

      int total = bc.pending + bc.accepted;
      int shortage = qMax(0, bc.required - bc.accepted);

      int statusIndex = managerStatusFilter ? managerStatusFilter->currentIndex() : 0;
      if ((statusIndex == 1 && shortage == 0) ||
          (statusIndex == 2 && bc.pending == 0) ||
          (statusIndex == 3 && shortage > 0))
        continue;

      // The compact manager card is intentionally rendered before the legacy
      // rich-text path below; the latter remains as a safe fallback for old
      // layouts but is unreachable for the current manager grid.
      const int percent = bc.required > 0
          ? qBound(0, qRound(100.0 * bc.accepted / bc.required), 100) : 100;
      const QString compactBg = shortage > 0 ? "#FEF2F2" : (bc.pending > 0 ? "#EFF6FF" : "#F0FDF4");
      const bool isSelected = col == m_selectedManagerDay && row == m_selectedManagerShift;
      const QString compactBorder = isSelected ? "#2563EB"
          : (shortage > 0 ? "#FECACA" : (bc.pending > 0 ? "#BFDBFE" : "#A7F3D0"));
      const QString accent = shortage > 0 ? "#DC2626" : (bc.pending > 0 ? "#2563EB" : "#059669");
      const QString status = shortage > 0 ? QString("! Thiếu %1").arg(shortage)
                                           : (bc.pending > 0 ? QString("%1 chờ duyệt").arg(bc.pending)
                                                             : "Đã đủ");
      QFrame *compactCard = new QFrame(ui->tableSum);
      compactCard->setAttribute(Qt::WA_TransparentForMouseEvents);
      compactCard->setStyleSheet(
          QString("QFrame { background:%1;border:%3px solid %2;border-radius:8px; }")
              .arg(compactBg, compactBorder).arg(isSelected ? 2 : 1));
      QVBoxLayout *cardLayout = new QVBoxLayout(compactCard);
      cardLayout->setContentsMargins(10, 10, 10, 10);
      cardLayout->setSpacing(7);
      cardLayout->setAlignment(Qt::AlignCenter);

      QLabel *ratioLabel = new QLabel(
          QString("%1 / %2 nhân viên").arg(bc.accepted).arg(bc.required), compactCard);
      ratioLabel->setAlignment(Qt::AlignCenter);
      ratioLabel->setStyleSheet("border:none;background:transparent;color:#334155;font-size:13px;font-weight:700;");

      QProgressBar *progress = new QProgressBar(compactCard);
      progress->setRange(0, 100);
      progress->setValue(percent);
      progress->setTextVisible(false);
      progress->setFixedHeight(7);
      progress->setStyleSheet(
          QString("QProgressBar { background:#E2E8F0;border:none;border-radius:3px; }"
                  "QProgressBar::chunk { background:%1;border-radius:3px; }").arg(accent));

      QLabel *statusLabel = new QLabel(status, compactCard);
      statusLabel->setAlignment(Qt::AlignCenter);
      statusLabel->setStyleSheet(
          QString("border:none;background:transparent;color:%1;font-size:11px;font-weight:700;").arg(accent));

      cardLayout->addStretch();
      cardLayout->addWidget(ratioLabel);
      cardLayout->addWidget(progress);
      cardLayout->addWidget(statusLabel);
      cardLayout->addStretch();
      compactCard->setToolTip(QString("%1/%2 đã xếp, %3 thiếu, %4 yêu cầu chờ duyệt")
                              .arg(bc.accepted).arg(bc.required).arg(shortage).arg(bc.pending));
      ui->tableSum->setCellWidget(row, col, compactCard);
      continue;

      // Determine total badge colour
      QString totalBadgeStyle;
      if (shortage > 0)
        totalBadgeStyle = "background-color:#FEE2E2;color:#991B1B";
      else if (bc.pending > 0)
        totalBadgeStyle = "background-color:#DBEAFE;color:#1D4ED8";
      else if (total == 0)
        totalBadgeStyle = "background-color:#F3F4F6;color:#6B7280";
      else
        totalBadgeStyle = "background-color:#D1FAE5;color:#065F46";

      QString cellBg = shortage > 0 ? "#FEF2F2" : (bc.pending > 0 ? "#EFF6FF" : "#F9FAFB");
      QString cellBorder = shortage > 0 ? "#FECACA" : (bc.pending > 0 ? "#BFDBFE" : "#E5E7EB");

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

      pills = QString("<div style='color:#374151;font-size:12px;font-weight:bold;'>Cần %1 · Đã xếp %2 · Thiếu %3</div>")
                  .arg(bc.required).arg(bc.accepted).arg(shortage);
      if (bc.pending > 0)
        pills += QString("<div style='color:#92400E;font-size:11px;margin-top:4px;'>%1 yêu cầu chờ xử lý</div>").arg(bc.pending);
      if (bc.declined > 0)
        pills += QString("<div style='color:#991B1B;font-size:10px;margin-top:2px;'>%1 từ chối</div>").arg(bc.declined);

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
    const QList<PendingShiftInfo> &requests, const QString &shiftLabel,
    const QList<EligibleEmployeeInfo> &eligibleEmployees,
    QDate shiftDate, QTime blockStart, QTime blockEnd)
{
  m_lastDrawerRequests = requests;
  m_lastDrawerEligible = eligibleEmployees;
  m_lastDrawerShiftLabel = shiftLabel;
  m_lastDrawerDate = shiftDate;
  m_lastDrawerStart = blockStart;
  m_lastDrawerEnd = blockEnd;

  if (shiftDetailDrawer && shiftDetailDrawerLayout)
  {
    clearLayoutItems(shiftDetailDrawerLayout);

    QStringList titleParts = shiftLabel.split(" — ");
    const QString shiftTitle = titleParts.value(0, shiftLabel);
    const QString shiftDateText = titleParts.size() > 1
                                      ? titleParts.last()
                                      : shiftDate.toString("dd/MM/yyyy");

    QHBoxLayout *titleRow = new QHBoxLayout();
    titleRow->setSpacing(10);
    QVBoxLayout *titleText = new QVBoxLayout();
    titleText->setSpacing(3);
    QLabel *eyebrow = new QLabel("Chi tiết ca làm", shiftDetailDrawer);
    eyebrow->setStyleSheet("color:#64748B;font-size:10px;font-weight:700;");
    QLabel *title = new QLabel(shiftTitle, shiftDetailDrawer);
    title->setWordWrap(true);
    title->setStyleSheet("color:#0F172A;font-size:17px;font-weight:800;");
    QLabel *dateLabel = new QLabel(shiftDateText, shiftDetailDrawer);
    dateLabel->setStyleSheet("color:#475569;font-size:11px;font-weight:600;");
    titleText->addWidget(eyebrow);
    titleText->addWidget(title);
    titleText->addWidget(dateLabel);
    QPushButton *closeButton = new QPushButton("×", shiftDetailDrawer);
    closeButton->setFixedSize(30, 30);
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setStyleSheet(
        "QPushButton { background:#F8FAFC;color:#64748B;border:1px solid #E2E8F0;"
        "border-radius:15px;font-size:18px;font-weight:700; }"
        "QPushButton:hover { background:#F1F5F9;color:#0F172A; }");
    connect(closeButton, &QPushButton::clicked, shiftDetailDrawer, &QWidget::hide);
    titleRow->addLayout(titleText, 1);
    titleRow->addWidget(closeButton, 0, Qt::AlignTop);
    shiftDetailDrawerLayout->addLayout(titleRow);

    int approvedCount = 0;
    int pendingCount = 0;
    for (const PendingShiftInfo &request : requests)
    {
      if (request.status == 1) ++approvedCount;
      else if (request.status == 0) ++pendingCount;
    }
    int required = 0;
    if (m_lastAssignCounts.contains(m_selectedManagerDay) &&
        m_lastAssignCounts[m_selectedManagerDay].contains(m_selectedManagerShift))
      required = m_lastAssignCounts[m_selectedManagerDay][m_selectedManagerShift].required;
    int missing = qMax(0, required - approvedCount);

    QHBoxLayout *metrics = new QHBoxLayout();
    metrics->setSpacing(8);
    auto makeMetric = [this](const QString &value, const QString &caption,
                             const QString &background, const QString &color) {
      QFrame *metric = new QFrame(shiftDetailDrawer);
      metric->setObjectName("shiftMetric");
      metric->setStyleSheet(
          QString("QFrame#shiftMetric { background:%1;border:none;border-radius:9px; }")
              .arg(background));
      QVBoxLayout *layout = new QVBoxLayout(metric);
      layout->setContentsMargins(8, 8, 8, 7);
      layout->setSpacing(1);
      QLabel *valueLabel = new QLabel(value, metric);
      valueLabel->setAlignment(Qt::AlignCenter);
      valueLabel->setStyleSheet(
          QString("color:%1;font-size:15px;font-weight:800;").arg(color));
      QLabel *captionLabel = new QLabel(caption, metric);
      captionLabel->setAlignment(Qt::AlignCenter);
      captionLabel->setStyleSheet("color:#64748B;font-size:9px;font-weight:600;");
      layout->addWidget(valueLabel);
      layout->addWidget(captionLabel);
      return metric;
    };
    metrics->addWidget(makeMetric(QString("%1/%2").arg(approvedCount).arg(required),
                                  "Đã xếp", "#EFF6FF", "#1D4ED8"));
    metrics->addWidget(makeMetric(QString::number(missing), "Còn thiếu",
                                  missing > 0 ? "#FEF2F2" : "#ECFDF5",
                                  missing > 0 ? "#DC2626" : "#059669"));
    metrics->addWidget(makeMetric(QString::number(pendingCount), "Chờ duyệt",
                                  "#FFFBEB", "#D97706"));
    shiftDetailDrawerLayout->addLayout(metrics);

    QScrollArea *scroll = new QScrollArea(shiftDetailDrawer);
    scroll->setObjectName("shiftDetailScroll");
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    scroll->setStyleSheet(
        "QScrollArea#shiftDetailScroll { background:#FFFFFF;border:none; }"
        "QScrollArea#shiftDetailScroll > QWidget > QWidget { background:#FFFFFF; }"
        "QScrollBar:vertical { background:#F8FAFC;width:7px;border-radius:3px; }"
        "QScrollBar::handle:vertical { background:#CBD5E1;border-radius:3px;min-height:28px; }"
        "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical { height:0; }");
    QWidget *scrollBody = new QWidget(scroll);
    scrollBody->setObjectName("shiftDetailScrollBody");
    scrollBody->setStyleSheet("QWidget#shiftDetailScrollBody { background:#FFFFFF; }");
    QVBoxLayout *bodyLayout = new QVBoxLayout(scrollBody);
    bodyLayout->setContentsMargins(0, 2, 5, 2);
    bodyLayout->setSpacing(10);

    QString selectedRole;
    switch (managerRoleFilter ? managerRoleFilter->currentIndex() : 0)
    {
    case 1: selectedRole = "Manager"; break;
    case 2: selectedRole = "Cashier"; break;
    case 3: selectedRole = "HallStaff"; break;
    case 4: selectedRole = "KitchenAssistant"; break;
    default: break;
    }
    auto matchesRole = [&selectedRole](const QString &role) {
      return selectedRole.isEmpty() || role.compare(selectedRole, Qt::CaseInsensitive) == 0 ||
             (selectedRole == "Manager" && role.compare("Manage", Qt::CaseInsensitive) == 0);
    };
    auto roleDisplayName = [](const QString &role) {
      if (role.compare("Manager", Qt::CaseInsensitive) == 0 ||
          role.compare("Manage", Qt::CaseInsensitive) == 0)
        return QString("Quản lý");
      if (role.compare("Cashier", Qt::CaseInsensitive) == 0)
        return QString("Thu ngân");
      if (role.compare("HallStaff", Qt::CaseInsensitive) == 0)
        return QString("Nhân viên sảnh");
      if (role.compare("KitchenAssistant", Qt::CaseInsensitive) == 0)
        return QString("Phụ bếp");
      return role;
    };
    auto sectionTitle = [scrollBody](const QString &text, int count) {
      QWidget *header = new QWidget(scrollBody);
      header->setStyleSheet("background:transparent;");
      QHBoxLayout *layout = new QHBoxLayout(header);
      layout->setContentsMargins(0, 7, 0, 1);
      QLabel *label = new QLabel(text, header);
      label->setStyleSheet("color:#334155;font-size:12px;font-weight:800;");
      QLabel *badge = new QLabel(QString::number(count), header);
      badge->setAlignment(Qt::AlignCenter);
      badge->setMinimumWidth(24);
      badge->setStyleSheet(
          "background:#F1F5F9;color:#475569;border-radius:10px;"
          "padding:2px 7px;font-size:10px;font-weight:700;");
      layout->addWidget(label);
      layout->addWidget(badge);
      layout->addStretch();
      return header;
    };
    auto addEmptyState = [scrollBody, bodyLayout](const QString &text) {
      QLabel *empty = new QLabel(text, scrollBody);
      empty->setWordWrap(true);
      empty->setAlignment(Qt::AlignCenter);
      empty->setStyleSheet(
          "background:#F8FAFC;color:#94A3B8;border:1px dashed #CBD5E1;"
          "border-radius:8px;padding:12px 10px;font-size:11px;");
      bodyLayout->addWidget(empty);
    };
    auto addEmployeeCard = [this, scrollBody, bodyLayout, roleDisplayName](
                               const PendingShiftInfo &info, bool pending) {
      QFrame *card = new QFrame(scrollBody);
      card->setObjectName("drawerEmployeeCard");
      card->setStyleSheet(
          "QFrame#drawerEmployeeCard { background:#FFFFFF;border:1px solid #E2E8F0;"
          "border-radius:9px; }"
          "QLabel { background:transparent;border:none; }");
      QHBoxLayout *cardLayout = new QHBoxLayout(card);
      cardLayout->setContentsMargins(10, 9, 9, 9);
      cardLayout->setSpacing(9);
      QLabel *avatar = new QLabel(info.employeeName.trimmed().left(1).toUpper(), card);
      avatar->setFixedSize(32, 32);
      avatar->setAlignment(Qt::AlignCenter);
      avatar->setStyleSheet(
          pending
              ? "background:#FEF3C7;color:#B45309;border-radius:16px;font-weight:800;"
              : "background:#DBEAFE;color:#1D4ED8;border-radius:16px;font-weight:800;");
      QVBoxLayout *employeeText = new QVBoxLayout();
      employeeText->setSpacing(2);
      QLabel *name = new QLabel(info.employeeName, card);
      name->setStyleSheet("color:#0F172A;font-size:12px;font-weight:700;");
      QLabel *meta = new QLabel(
          QString("%1 · %2–%3").arg(roleDisplayName(info.role),
              info.startTime.toString("HH:mm"), info.endTime.toString("HH:mm")), card);
      meta->setStyleSheet("color:#64748B;font-size:11px;");
      employeeText->addWidget(name);
      employeeText->addWidget(meta);
      cardLayout->addWidget(avatar);
      cardLayout->addLayout(employeeText, 1);
      QHBoxLayout *actions = new QHBoxLayout();
      actions->setSpacing(5);
      if (pending)
      {
        QPushButton *decline = new QPushButton("Từ chối", card);
        QPushButton *approve = new QPushButton("Duyệt", card);
        decline->setStyleSheet("QPushButton { background:#FFFFFF;color:#B91C1C;border:1px solid #FECACA;border-radius:6px;padding:5px 8px;font-weight:700; } QPushButton:hover { background:#FEF2F2; }");
        approve->setStyleSheet("QPushButton { background:#16A34A;color:white;border:none;border-radius:6px;padding:6px 9px;font-weight:700; } QPushButton:hover { background:#15803D; }");
        connect(decline, &QPushButton::clicked, this,
                [this, info, decline, approve]() {
                  decline->setEnabled(false);
                  approve->setEnabled(false);
                  emit requestDeclineShift(info);
                });
        connect(approve, &QPushButton::clicked, this,
                [this, info, decline, approve]() {
                  decline->setEnabled(false);
                  approve->setEnabled(false);
                  emit requestApproveShift(info);
                });
        actions->addWidget(decline);
        actions->addWidget(approve);
      }
      else
      {
        QPushButton *remove = new QPushButton("Gỡ", card);
        remove->setToolTip("Gỡ nhân viên khỏi ca");
        remove->setStyleSheet("QPushButton { background:#FFF7ED;color:#C2410C;border:1px solid #FED7AA;border-radius:6px;padding:5px 10px;font-weight:700; } QPushButton:hover { background:#FFEDD5; }");
        connect(remove, &QPushButton::clicked, this, [this, info, remove]() {
          bool ok = false;
          QString reason = QInputDialog::getText(
              shiftDetailDrawer, "Gỡ nhân viên khỏi ca", "Lý do:",
              QLineEdit::Normal, "Điều chỉnh phân ca", &ok);
          if (ok && !reason.trimmed().isEmpty())
          {
            remove->setEnabled(false);
            remove->setText("Đã gỡ");
            emit requestRemoveAssignedShift(info.shiftId, info.employeeId, reason.trimmed());
          }
        });
        actions->addWidget(remove);
      }
      cardLayout->addLayout(actions, 0);
      bodyLayout->addWidget(card);
    };

    int visibleApproved = 0;
    int visiblePending = 0;
    for (const PendingShiftInfo &request : requests)
    {
      if (!matchesRole(request.role)) continue;
      if (request.status == 1) ++visibleApproved;
      else if (request.status == 0) ++visiblePending;
    }

    bodyLayout->addWidget(sectionTitle("Nhân viên đã xếp", visibleApproved));
    for (const PendingShiftInfo &request : requests)
      if (request.status == 1 && matchesRole(request.role))
        addEmployeeCard(request, false);
    if (visibleApproved == 0)
      addEmptyState("Chưa có nhân viên phù hợp với bộ lọc.");

    bodyLayout->addWidget(sectionTitle("Yêu cầu chờ duyệt", visiblePending));
    for (const PendingShiftInfo &request : requests)
      if (request.status == 0 && matchesRole(request.role))
        addEmployeeCard(request, true);
    if (visiblePending == 0)
      addEmptyState("Không có yêu cầu nào đang chờ duyệt.");

    QFrame *addStaffPanel = new QFrame(scrollBody);
    addStaffPanel->setObjectName("addStaffPanel");
    addStaffPanel->setStyleSheet(
        "QFrame#addStaffPanel { background:#F8FAFC;border:1px solid #E2E8F0;"
        "border-radius:10px; } QLabel { background:transparent;border:none; }");
    QVBoxLayout *addStaffLayout = new QVBoxLayout(addStaffPanel);
    addStaffLayout->setContentsMargins(11, 10, 11, 11);
    addStaffLayout->setSpacing(8);
    QLabel *addStaffTitle = new QLabel("Bổ sung nhân sự", addStaffPanel);
    addStaffTitle->setStyleSheet("color:#334155;font-size:12px;font-weight:800;");
    QLabel *addStaffHint = new QLabel("Chỉ hiển thị nhân viên khả dụng cho ca này.", addStaffPanel);
    addStaffHint->setWordWrap(true);
    addStaffHint->setStyleSheet("color:#64748B;font-size:10px;");
    addStaffLayout->addWidget(addStaffTitle);
    addStaffLayout->addWidget(addStaffHint);

    QComboBox *eligibleCombo = new QComboBox(addStaffPanel);
    QList<EligibleEmployeeInfo> eligibleVisible;
    int incompatibleCount = 0;
    for (const EligibleEmployeeInfo &employee : eligibleEmployees)
    {
      if (!matchesRole(employee.role)) continue;
      if (employee.eligible)
      {
        eligibleVisible.append(employee);
        eligibleCombo->addItem(
            QString("%1 — %2").arg(employee.employeeName,
                                      roleDisplayName(employee.role)));
      }
      else
      {
        ++incompatibleCount;
      }
    }
    eligibleCombo->setStyleSheet(
        "QComboBox { background:#FFFFFF;border:1px solid #CBD5E1;border-radius:6px;"
        "padding:8px;color:#334155; }"
        "QComboBox:focus { border:1px solid #60A5FA; }");
    if (eligibleVisible.isEmpty())
      eligibleCombo->addItem("Không có nhân viên khả dụng");
    addStaffLayout->addWidget(eligibleCombo);
    if (incompatibleCount > 0)
    {
      QLabel *conflicts = new QLabel(
          QString("%1 nhân viên không phù hợp do trùng ca hoặc không khả dụng.")
              .arg(incompatibleCount), addStaffPanel);
      conflicts->setWordWrap(true);
      conflicts->setStyleSheet(
          "background:#FFFBEB;color:#B45309;border-radius:6px;"
          "padding:6px 8px;font-size:10px;");
      addStaffLayout->addWidget(conflicts);
    }
    QPushButton *addButton = new QPushButton("+ Thêm nhân viên vào ca", addStaffPanel);
    addButton->setEnabled(!eligibleVisible.isEmpty());
    addButton->setStyleSheet(
        "QPushButton { background:#2563EB;color:white;border:none;border-radius:6px;"
        "padding:9px 12px;font-weight:700; }"
        "QPushButton:hover { background:#1D4ED8; }"
        "QPushButton:disabled { background:#E2E8F0;color:#94A3B8; }");
    connect(addButton, &QPushButton::clicked, this,
            [this, eligibleCombo, eligibleVisible, shiftDate, blockStart, blockEnd, addButton]() {
      int index = eligibleCombo->currentIndex();
      if (index < 0 || index >= eligibleVisible.size()) return;
      const EligibleEmployeeInfo employee = eligibleVisible[index];
      QTime addStart = blockStart;
      QTime addEnd = blockEnd;
      if (!employee.isFixedSalary)
      {
        bool startOk = false;
        bool endOk = false;
        QString startText = QInputDialog::getText(
            shiftDetailDrawer, "Giờ bắt đầu", "Giờ bắt đầu (HH:mm):",
            QLineEdit::Normal, blockStart.toString("HH:mm"), &startOk);
        if (!startOk) return;
        QString endText = QInputDialog::getText(
            shiftDetailDrawer, "Giờ kết thúc", "Giờ kết thúc (HH:mm):",
            QLineEdit::Normal, blockEnd.toString("HH:mm"), &endOk);
        if (!endOk) return;
        addStart = QTime::fromString(startText.trimmed(), "HH:mm");
        addEnd = QTime::fromString(endText.trimmed(), "HH:mm");
        if (!addStart.isValid() || !addEnd.isValid() || addStart >= addEnd)
        {
          QMessageBox::warning(shiftDetailDrawer, "Khoảng giờ không hợp lệ",
                               "Giờ bắt đầu phải nhỏ hơn giờ kết thúc.");
          return;
        }
      }
      emit requestAddEmployee(employee.employeeId, shiftDate, addStart, addEnd,
                              employee.isFixedSalary
                                  ? "Quản lý bổ sung nhân sự cố định"
                                  : "Quản lý bổ sung nhân sự theo giờ");
      addButton->setText("✓ Đã thêm vào bản nháp");
      addButton->setEnabled(false);
    });
    addStaffLayout->addWidget(addButton);
    bodyLayout->addWidget(addStaffPanel);
    bodyLayout->addStretch();
    scroll->setWidget(scrollBody);
    shiftDetailDrawerLayout->addWidget(scroll, 1);
    shiftDetailDrawer->setVisible(true);
    return;
  }

  QDialog *dlg = new QDialog(this);
  dlg->setWindowTitle(QString("Yêu cầu — %1").arg(shiftLabel));
  dlg->setMinimumWidth(720);
  dlg->setMinimumHeight(400);
  dlg->setWindowFlags(Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
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
    QString roleDisplay = "Nhân viên";
    QString roleColors = "background-color:#DBEAFE;color:#1D4ED8;";
    if (info.role == "Manager")
    {
      roleDisplay = "Quản lý";
      roleColors = "background-color:#EDE9FE;color:#6D28D9;";
    }
    else if (info.role == "Admin")
    {
      roleDisplay = "Quản trị viên";
      roleColors = "background-color:#DBEAFE;color:#1D4ED8;";
    }
    else if (info.role == "Cashier")
    {
      roleDisplay = "Thu ngân";
      roleColors = "background-color:#CCFBF1;color:#0F766E;";
    }
    else if (info.role == "HallStaff")
    {
      roleDisplay = "Nhân viên sảnh";
      roleColors = "background-color:#FFEDD5;color:#C2410C;";
    }
    else if (info.role == "KitchenAssistant")
    {
      roleDisplay = "Phụ bếp";
      roleColors = "background-color:#FFE4E6;color:#BE123C;";
    }
    QString roleStyle = roleColors +
        "border-radius:4px;padding:2px 8px;font-weight:bold;font-size:12px;";
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
      connect(btnApprove, &QPushButton::clicked, this, [this, info, dlg]()
              {
        emit requestApproveShift(info);
        dlg->accept(); });
      connect(btnDecline, &QPushButton::clicked, this, [this, info, dlg]()
              {
        emit requestDeclineShift(info);
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
      QPushButton *btnRemove = new QPushButton("Gỡ khỏi ca", actionCell);
      btnRemove->setStyleSheet("QPushButton { background:#F97316;color:white;border-radius:4px;padding:4px 8px;font-weight:bold;font-size:11px; }");
      connect(btnRemove, &QPushButton::clicked, this, [this, info, dlg]()
      {
        bool ok = false;
        QString reason = QInputDialog::getText(dlg, "Gỡ nhân viên khỏi ca",
                                               "Lý do:", QLineEdit::Normal,
                                               "Điều chỉnh phân ca", &ok);
        if (ok && !reason.trimmed().isEmpty())
        {
          emit requestRemoveAssignedShift(info.shiftId, info.employeeId, reason.trimmed());
          dlg->accept();
        }
      });
      actionLayout->addWidget(badge);
      actionLayout->addWidget(btnRemove);
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

  QPushButton *btnAddEmployee = new QPushButton("+ Thêm nhân viên", dlg);
  btnAddEmployee->setStyleSheet("QPushButton { background:#2563EB;color:white;border-radius:6px;padding:7px 14px;font-weight:bold; }");
  connect(btnAddEmployee, &QPushButton::clicked, this,
          [this, dlg, requests, eligibleEmployees, shiftDate, blockStart, blockEnd]()
  {
    if (!shiftDate.isValid() || !blockStart.isValid() || !blockEnd.isValid())
    {
      QMessageBox::information(dlg, "Thêm nhân viên",
                               "Ca này chưa có yêu cầu để lấy khung giờ. Hãy tạo yêu cầu trước hoặc dùng phân ca tự động.");
      return;
    }
    QStringList choices;
    for (const EligibleEmployeeInfo &employee : eligibleEmployees)
    {
      if (!employee.eligible) continue;
      choices << QString("%1 — %2 (%3)").arg(employee.employeeName)
                                           .arg(employee.role)
                                           .arg(employee.isFixedSalary ? "Lương cố định" : "Lương giờ");
    }
    if (choices.isEmpty())
    {
      QMessageBox::information(dlg, "Thêm nhân viên", "Không có nhân viên đủ điều kiện cho ca này.");
      return;
    }
    bool ok = false;
    QString selected = QInputDialog::getItem(dlg, "Thêm nhân viên vào ca",
                                             "Nhân viên đủ điều kiện:", choices, 0, false, &ok);
    if (!ok) return;
    int selectedIndex = choices.indexOf(selected);
    QList<EligibleEmployeeInfo> eligibleOnly;
    for (const EligibleEmployeeInfo &employee : eligibleEmployees)
      if (employee.eligible) eligibleOnly.append(employee);
    if (selectedIndex < 0 || selectedIndex >= eligibleOnly.size()) return;
    const EligibleEmployeeInfo &selectedEmployee = eligibleOnly[selectedIndex];
    QTime addStart = blockStart;
    QTime addEnd = blockEnd;
    if (!selectedEmployee.isFixedSalary)
    {
      bool startOk = false;
      bool endOk = false;
      QString startText = QInputDialog::getText(dlg, "Giờ bắt đầu",
                                                "Nhập giờ bắt đầu (HH:mm):",
                                                QLineEdit::Normal,
                                                blockStart.toString("HH:mm"), &startOk);
      if (!startOk) return;
      QString endText = QInputDialog::getText(dlg, "Giờ kết thúc",
                                              "Nhập giờ kết thúc (HH:mm):",
                                              QLineEdit::Normal,
                                              blockEnd.toString("HH:mm"), &endOk);
      if (!endOk) return;
      addStart = QTime::fromString(startText.trimmed(), "HH:mm");
      addEnd = QTime::fromString(endText.trimmed(), "HH:mm");
      if (!addStart.isValid() || !addEnd.isValid() || addStart >= addEnd)
      {
        QMessageBox::warning(dlg, "Khoảng giờ không hợp lệ", "Giờ bắt đầu phải nhỏ hơn giờ kết thúc.");
        return;
      }
    }
    emit requestAddEmployee(selectedEmployee.employeeId, shiftDate,
                            addStart, addEnd,
                            selectedEmployee.isFixedSalary
                                ? "Quản lý bổ sung nhân sự cố định"
                                : "Quản lý bổ sung nhân sự theo giờ");
    dlg->accept();
  });
  QHBoxLayout *addRow = new QHBoxLayout();
  addRow->addWidget(btnAddEmployee);
  addRow->addStretch();
  mainLayout->addLayout(addRow);

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

  dlg->adjustSize();
  if (parentWidget())
    dlg->move(parentWidget()->mapToGlobal(QPoint(parentWidget()->width() - dlg->width() - 24, 70)));
  dlg->setAttribute(Qt::WA_DeleteOnClose, true);
  dlg->show();
  // WA_DeleteOnClose owns the non-blocking detail surface.
}

void Schedule_View::setManagerDraftStatus(int changeCount)
{
  if (lblManagerDraft) lblManagerDraft->setText(QString::number(changeCount));
  if (managerUndoDraftButton) managerUndoDraftButton->setEnabled(changeCount > 0);
  if (managerClearDraftButton) managerClearDraftButton->setEnabled(changeCount > 0);
  if (!lblManagerDraftStatus) return;
  if (changeCount > 0)
  {
    lblManagerDraftStatus->setText(QString("%1 thay đổi chưa lưu").arg(changeCount));
    lblManagerDraftStatus->setStyleSheet("background:#FEF3C7;color:#92400E;border-radius:8px;padding:8px 12px;font-weight:600;");
  }
  else
  {
    lblManagerDraftStatus->setText("Đã xác nhận");
    lblManagerDraftStatus->setStyleSheet("background:#ECFDF5;color:#047857;border-radius:8px;padding:8px 12px;font-weight:600;");
  }
}

void Schedule_View::updateManagerSummary(int totalShifts, int shortageShifts,
                                          int pendingRequests, int draftChanges,
                                          int missingSlots, int staffedShifts)
{
  if (lblManagerTotal) lblManagerTotal->setText(
      QString("%1 ca / %2 đã đủ").arg(totalShifts).arg(staffedShifts));
  if (lblManagerShortage) lblManagerShortage->setText(
      QString("%1 ca / %2 vị trí").arg(shortageShifts).arg(missingSlots));
  if (lblManagerPending) lblManagerPending->setText(QString::number(pendingRequests));
  if (lblManagerDraft) lblManagerDraft->setText(QString::number(draftChanges));
  if (lblManagerSummary)
    lblManagerSummary->setText(QString("Tổng số ca: %1   |   Ca thiếu nhân sự: %2   |   Yêu cầu chờ xử lý: %3")
                                   .arg(totalShifts).arg(shortageShifts).arg(pendingRequests));
  setManagerDraftStatus(draftChanges);
}

void Schedule_View::updateManagerWeek(QDate monday)
{
  if (!lblManagerWeek || !monday.isValid()) return;
  lblManagerWeek->setText(QString("%1 - %2")
      .arg(monday.toString("dd/MM/yyyy"), monday.addDays(6).toString("dd/MM/yyyy")));
}

void Schedule_View::updateManagerMissingShifts(
    const QList<MissingShiftInfo> &missingList)
{
  if (!tableMissingStaff)
    return;

  tableMissingStaff->setSortingEnabled(false);

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
    int deficit = qMax(0, info.required - info.assigned);
    const int coverage = info.required > 0
        ? qBound(0, qRound(100.0 * info.assigned / info.required), 100) : 100;
    const double deficitRatio = info.required > 0
        ? static_cast<double>(deficit) / info.required : 0.0;
    tableMissingStaff->setRowHeight(i, 68);

    for (int column = 0; column < tableMissingStaff->columnCount(); ++column)
    {
      QTableWidgetItem *backgroundItem = new QTableWidgetItem();
      backgroundItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      tableMissingStaff->setItem(i, column, backgroundItem);
    }

    QString severityText;
    QString severityBg;
    QString severityColor;
    if (deficitRatio >= 0.75)
    {
      severityText = "Khẩn cấp";
      severityBg = "#FEE2E2";
      severityColor = "#B91C1C";
    }
    else if (deficitRatio >= 0.4)
    {
      severityText = "Thiếu nhiều";
      severityBg = "#FFEDD5";
      severityColor = "#C2410C";
    }
    else
    {
      severityText = "Cần bổ sung";
      severityBg = "#FEF3C7";
      severityColor = "#A16207";
    }

    QWidget *severityCell = new QWidget(tableMissingStaff);
    QHBoxLayout *severityLayout = new QHBoxLayout(severityCell);
    severityLayout->setContentsMargins(12, 14, 12, 14);
    QLabel *severityBadge = new QLabel(severityText, severityCell);
    severityBadge->setAlignment(Qt::AlignCenter);
    severityBadge->setStyleSheet(
        QString("background:%1;color:%2;border-radius:10px;padding:5px 10px;"
                "font-size:11px;font-weight:700;").arg(severityBg, severityColor));
    severityLayout->addWidget(severityBadge);
    tableMissingStaff->setCellWidget(i, 0, severityCell);

    QWidget *shiftCell = new QWidget(tableMissingStaff);
    QVBoxLayout *shiftLayout = new QVBoxLayout(shiftCell);
    shiftLayout->setContentsMargins(14, 8, 14, 8);
    shiftLayout->setSpacing(2);
    QLabel *dateLabel = new QLabel(info.dateStr, shiftCell);
    dateLabel->setStyleSheet("color:#0F172A;font-size:13px;font-weight:700;");
    QLabel *shiftLabel = new QLabel(info.shiftName, shiftCell);
    shiftLabel->setStyleSheet("color:#2563EB;font-size:12px;font-weight:600;");
    shiftLayout->addWidget(dateLabel);
    shiftLayout->addWidget(shiftLabel);
    tableMissingStaff->setCellWidget(i, 1, shiftCell);

    QWidget *staffingCell = new QWidget(tableMissingStaff);
    QVBoxLayout *staffingLayout = new QVBoxLayout(staffingCell);
    staffingLayout->setContentsMargins(14, 9, 14, 9);
    staffingLayout->setSpacing(6);
    QLabel *staffingLabel = new QLabel(
        QString("%1 / %2 nhân viên đã xếp").arg(info.assigned).arg(info.required),
        staffingCell);
    staffingLabel->setStyleSheet("color:#334155;font-size:12px;font-weight:600;");
    QProgressBar *staffingProgress = new QProgressBar(staffingCell);
    staffingProgress->setRange(0, 100);
    staffingProgress->setValue(coverage);
    staffingProgress->setTextVisible(false);
    staffingProgress->setFixedHeight(7);
    staffingProgress->setStyleSheet(
        "QProgressBar { background:#E2E8F0;border:none;border-radius:3px; }"
        "QProgressBar::chunk { background:#EF4444;border-radius:3px; }");
    staffingLayout->addWidget(staffingLabel);
    staffingLayout->addWidget(staffingProgress);
    tableMissingStaff->setCellWidget(i, 2, staffingCell);

    QWidget *deficitCell = new QWidget(tableMissingStaff);
    QHBoxLayout *deficitLayout = new QHBoxLayout(deficitCell);
    deficitLayout->setContentsMargins(12, 12, 12, 12);
    QLabel *deficitBadge = new QLabel(QString("-%1 vị trí").arg(deficit), deficitCell);
    deficitBadge->setAlignment(Qt::AlignCenter);
    deficitBadge->setStyleSheet(
        "background:#FEF2F2;color:#DC2626;border:1px solid #FECACA;"
        "border-radius:8px;padding:6px 10px;font-size:12px;font-weight:800;");
    deficitLayout->addWidget(deficitBadge);
    tableMissingStaff->setCellWidget(i, 3, deficitCell);

    QWidget *actionCell = new QWidget(tableMissingStaff);
    QHBoxLayout *actionLayout = new QHBoxLayout(actionCell);
    actionLayout->setContentsMargins(10, 14, 10, 14);
    QPushButton *resolveButton = new QPushButton("Xử lý ca  →", actionCell);
    resolveButton->setMinimumHeight(34);
    resolveButton->setCursor(Qt::PointingHandCursor);
    resolveButton->setStyleSheet(
        "QPushButton { background:#2563EB;color:white;border:none;border-radius:6px;"
        "padding:7px 14px;font-weight:700; }"
        "QPushButton:hover { background:#1D4ED8; }"
        "QPushButton:pressed { background:#1E40AF; }");
    actionLayout->addWidget(resolveButton);
    connect(resolveButton, &QPushButton::clicked, this, [this, info]() {
      if (info.dayColumn >= 0 && info.shiftRow >= 0)
      {
        selectManagerShift(info.dayColumn, info.shiftRow);
        emit shiftBlockClicked(info.dayColumn, info.shiftRow);
      }
    });
    tableMissingStaff->setCellWidget(i, 4, actionCell);
  }
}

