#include "Schedule_View.h"
#include "global.h"
#include "ui_Schedule_View.h"
#include <QProgressBar>

namespace {
static const QString SHIFT_NAMES[3] = {"Ca Sáng", "Ca Chiều", "Ca Tối"};
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
    const bool compactChrome = width() > 0 && width() < 1650;
    if (QWidget *toolbar = findChild<QWidget *>("managerToolbar"))
    {
      toolbar->setVisible(true);
      toolbar->setStyleSheet(compactChrome
                                 ? "QComboBox { font-size:10px; padding:4px 6px; } QPushButton { font-size:10px; padding:4px 7px; }"
                                 : QString());
    }
    if (QWidget *summaryBar = findChild<QWidget *>("managerSummaryBar"))
    {
      summaryBar->setVisible(true);
      for (QLabel *label : summaryBar->findChildren<QLabel *>())
      {
        if (label->objectName() == "managerMetricTitle")
        {
          QString style = label->styleSheet();
          style.replace(QRegularExpression("font-size:\\d+px"),
                        compactChrome ? "font-size:9px" : "font-size:11px");
          label->setStyleSheet(style);
        }
        else if (label->objectName() == "managerMetricValue")
        {
          QString style = label->styleSheet();
          style.replace(QRegularExpression("font-size:\\d+px"),
                        compactChrome ? "font-size:17px" : "font-size:20px");
          label->setStyleSheet(style);
        }
      }
    }
    if (staffInfoStack)
      staffInfoStack->setVisible(false);
    if (lblFullTimeFooterMessage)
      lblFullTimeFooterMessage->setVisible(false);
    if (lblPartTimeFooterMessage)
      lblPartTimeFooterMessage->setVisible(false);
    ui->DangKyLich->setVisible(false);
    ui->tableInteractiveGrid->setVisible(false);
    ui->frameDangKyContainer->setVisible(false);
    ui->buttonLuu->setVisible(false);
    ui->buttonClearPending->setVisible(false);
    if (requestLeaveButton)
      requestLeaveButton->setVisible(false);
    if (leaveHistoryButton)
      leaveHistoryButton->setVisible(false);

    ui->btnGenSchedule->setVisible(true);
    ui->btnGenSchedule->setEnabled(true);
    ui->btnConfirm->setVisible(true);
    ui->btnConfirm->setEnabled(true);
    ui->btnConfirm->setText("Xem lại & công bố");
    ui->XacNhanLich->setVisible(true);
    ui->frameTableContainer->setVisible(true);
    ui->XacNhanLich->setText("TỔNG KẾT YÊU CẦU ĐĂNG KÝ TRONG TUẦN");
    ui->XacNhanLich->setStyleSheet(
        QString("color:#1F2937;font-size:%1px;font-weight:700;padding:2px 0 4px 0;")
            .arg(compactChrome ? 16 : 18));

    // 3-shift manager grid
    ui->tableSum->setRowCount(3);
    ui->tableSum->setColumnCount(7);
    QStringList shiftLabels = {
        QString("%1\n%2").arg(SHIFT_NAMES[0], Config::getShiftTimeLabel(0)),
        QString("%1\n%2").arg(SHIFT_NAMES[1], Config::getShiftTimeLabel(1)),
        QString("%1\n%2").arg(SHIFT_NAMES[2], Config::getShiftTimeLabel(2))};
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
    ui->buttonClearPending->setVisible(true);
    if (requestLeaveButton)
      requestLeaveButton->setVisible(true);
    if (leaveHistoryButton)
      leaveHistoryButton->setVisible(true);

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

void Schedule_View::setManagerAssignmentState(bool isOpen, QDate nextOpenDate)
{
  m_managerAssignmentOpen = isOpen;
  const QString tooltip = isOpen
      ? QString()
      : QString::fromUtf8("Xếp lịch chỉ mở vào ngày đã cấu hình%1.")
            .arg(nextOpenDate.isValid()
                     ? QString::fromUtf8(" (%1)")
                           .arg(nextOpenDate.toString("dd/MM/yyyy"))
                     : QString());

  ui->tableSum->setEnabled(isOpen);
  ui->tableSum->setToolTip(tooltip);
  ui->btnGenSchedule->setEnabled(isOpen);
  ui->btnGenSchedule->setToolTip(tooltip);
  ui->btnConfirm->setEnabled(isOpen);
  ui->btnConfirm->setToolTip(tooltip);
  if (missingStaffWidget)
  {
    missingStaffWidget->setEnabled(isOpen);
    missingStaffWidget->setToolTip(tooltip);
  }
  if (activeManagerAddButton)
    activeManagerAddButton->setEnabled(isOpen &&
                                       !m_managerEmployeeSelections.isEmpty());
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
  // At Windows 125% scaling the table receives fewer logical pixels even on a
  // 1920px-wide monitor. Use a compact card presentation only in that case;
  // the normal 2K/100% layout keeps the more descriptive labels.
  const int tableWidth = ui->tableSum->viewport()->width();
  const bool compactCards = tableWidth > 0 && tableWidth < 900;
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
      compactCard->setMinimumSize(0, 0);
      compactCard->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
      compactCard->setStyleSheet(
          QString("QFrame { background:%1;border:%3px solid %2;border-radius:8px; }")
              .arg(compactBg, compactBorder).arg(isSelected ? 2 : 1));
      QVBoxLayout *cardLayout = new QVBoxLayout(compactCard);
      const int cardMargin = compactCards ? 5 : 10;
      cardLayout->setContentsMargins(cardMargin, cardMargin, cardMargin,
                                     cardMargin);
      cardLayout->setSpacing(compactCards ? 4 : 7);
      cardLayout->setAlignment(Qt::AlignCenter);

      QLabel *ratioLabel = new QLabel(
          compactCards
              ? QString("%1 / %2").arg(bc.accepted).arg(bc.required)
              : QString("%1 / %2 nhân viên").arg(bc.accepted).arg(bc.required),
          compactCard);
      ratioLabel->setAlignment(Qt::AlignCenter);
      if (compactCards)
        ratioLabel->setText(QString("%1 / %2 %3")
                                .arg(bc.accepted)
                                .arg(bc.required)
                                .arg(QString::fromUtf8("nhân viên")));
      ratioLabel->setWordWrap(!compactCards);
      ratioLabel->setMinimumWidth(0);
      ratioLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
      ratioLabel->setStyleSheet(QString(
          "border:none;background:transparent;color:#334155;font-size:%1px;font-weight:700;")
                                    .arg(compactCards ? 10 : 13));

      QProgressBar *progress = new QProgressBar(compactCard);
      progress->setRange(0, 100);
      progress->setValue(percent);
      progress->setTextVisible(false);
      progress->setFixedHeight(compactCards ? 5 : 7);
      progress->setStyleSheet(
          QString("QProgressBar { background:#E2E8F0;border:none;border-radius:3px; }"
                  "QProgressBar::chunk { background:%1;border-radius:3px; }").arg(accent));

      QLabel *statusLabel = new QLabel(status, compactCard);
      statusLabel->setAlignment(Qt::AlignCenter);
      statusLabel->setStyleSheet(
          QString("border:none;background:transparent;color:%1;font-size:%2px;font-weight:700;")
              .arg(accent)
              .arg(compactCards ? 10 : 11));

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


void Schedule_View::setManagerDraftStatus(int changeCount)
{
  if (lblManagerDraft) lblManagerDraft->setText(QString::number(changeCount));
  if (managerUndoDraftButton)
    managerUndoDraftButton->setEnabled(m_managerAssignmentOpen && changeCount > 0);
  if (managerClearDraftButton)
    managerClearDraftButton->setEnabled(m_managerAssignmentOpen && changeCount > 0);
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

void Schedule_View::resetManagerAddButton()
{
  managerAddRejectedDuringRequest = true;
  if (!activeManagerAddButton)
    return;
  activeManagerAddButton->setText("+ Thêm nhân viên vào ca");
  activeManagerAddButton->setEnabled(m_managerAssignmentOpen);
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
