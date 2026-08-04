#include "Schedule_View.h"
#include "global.h"
#include "ui_Schedule_View.h"
#include "utils/Config.h"

namespace {
static const QString SHIFT_NAMES[3] = {"Ca Sáng", "Ca Chiều", "Ca Tối"};
static const QString SHIFT_TIMES[3] = {"07:00 - 12:00", "12:00 - 17:00",
                                       "17:00 - 22:00"};
static const QColor SELECTED_COLOR(0xBF, 0xDB, 0xFE);
static const QColor SELECTED_BORDER(0x1D, 0x4E, 0xD8);

constexpr int FULL_TIME_VISUAL_ROLE = Qt::UserRole + 1;
constexpr int PART_TIME_VISUAL_ROLE = Qt::UserRole + 2;
constexpr int PART_TIME_MUTED_ROLE = Qt::UserRole + 3;
constexpr int SHIFT_BOUNDARY_ROLE = Qt::UserRole + 4;

enum FullTimeVisualState {
  VisualUnregistered = 0,
  VisualRegistered,
  VisualApproved,
  VisualDeclined,
  VisualSelected,
  VisualPendingRemoval,
  VisualStaffShortage
};

enum PartTimeVisualState {
  HourlyAvailable = 0,
  HourlyUnderstaffed,
  HourlySufficient,
  HourlyPending,
  HourlyApproved
};
} // namespace

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
    QString hourLabel = QString("%1:00 - %2:00")
                            .arg(h, 2, 10, QChar('0'))
                            .arg(h + 1, 2, 10, QChar('0'));
    if (h == 7)
      hourLabel = QString("Ca Sáng\n%1").arg(hourLabel);
    else if (h == 12)
      hourLabel = QString("Ca Chiều\n%1").arg(hourLabel);
    else if (h == 17)
      hourLabel = QString("Ca Tối\n%1").arg(hourLabel);
    rowLabels << hourLabel;
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
    bool shiftBoundary = (r == 0 || r == 5 || r == 10);
    grid->setRowHeight(r, shiftBoundary ? 48 : 38);
    if (QTableWidgetItem *headerItem = grid->verticalHeaderItem(r))
    {
      QFont headerFont = headerItem->font();
      headerFont.setBold(shiftBoundary);
      headerItem->setFont(headerFont);
      headerItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }
    for (int c = 0; c < 7; ++c)
    {
      QTableWidgetItem *item = new QTableWidgetItem();
      item->setTextAlignment(Qt::AlignCenter);
      item->setFlags(m_partTimeRegistrationOpen
                         ? Qt::ItemIsEnabled | Qt::ItemIsSelectable
                         : Qt::NoItemFlags);
      item->setData(PART_TIME_VISUAL_ROLE, HourlyAvailable);
      item->setData(PART_TIME_MUTED_ROLE, !m_partTimeRegistrationOpen);
      item->setData(SHIFT_BOUNDARY_ROLE, shiftBoundary);
      item->setToolTip("Chưa có dữ liệu nhân sự");
      grid->setItem(r, c, item);
    }
  }

  grid->setMinimumHeight(650);
  grid->setMaximumHeight(700);
  grid->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  ui->frameDangKyContainer->setMinimumHeight(650);
  ui->frameDangKyContainer->setMaximumHeight(700);
  ui->frameDangKyContainer->setSizePolicy(QSizePolicy::Expanding,
                                          QSizePolicy::Expanding);
}

void Schedule_View::setUpInteractiveGrid(QDate weekStart, int openTime, int closeTime)
{
  m_isFullTimeMode = false;
  m_partTimeDragActive = false;
  m_partTimeDragVisited.clear();
  m_partTimePendingCells.clear();
  m_partTimeApprovedDays.clear();
  m_fullTimeStatuses.clear();
  m_fullTimeSelections.clear();
  fullTimeInfoWidget->setVisible(false);
  lblFullTimeFooterMessage->setVisible(false);
  partTimeInfoWidget->setVisible(true);
  lblPartTimeFooterMessage->setVisible(true);
  m_openHour  = openTime;
  m_closeHour = closeTime;

  ui->tableInteractiveGrid->setSelectionMode(QAbstractItemView::MultiSelection);
  ui->tableInteractiveGrid->setSelectionBehavior(QAbstractItemView::SelectItems);
  ui->tableInteractiveGrid->verticalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  ui->tableInteractiveGrid->verticalHeader()->setMinimumSectionSize(38);
  ui->tableInteractiveGrid->verticalHeader()->setDefaultAlignment(
      Qt::AlignCenter);
  ui->tableInteractiveGrid->verticalHeader()->setMinimumWidth(165);
  ui->tableInteractiveGrid->verticalHeader()->setMaximumWidth(165);
  ui->tableInteractiveGrid->horizontalHeader()->setStyleSheet(
      "QHeaderView::section { background-color:#1D4ED8;color:white;"
      "font-weight:bold;padding:6px;border:none; }");
  ui->tableInteractiveGrid->verticalHeader()->setStyleSheet(
      "QHeaderView::section {"
      "background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
      "stop:0 #2563EB,stop:0.035 #2563EB,stop:0.036 #F8FAFC,stop:1 #F8FAFC);"
      "color:#334155;font-size:11px;padding:6px 10px;border:none;"
      "border-right:1px solid #CBD5E1;border-bottom:1px solid #E2E8F0; }");
  ui->tableInteractiveGrid->horizontalHeader()->setMinimumHeight(0);
  ui->tableInteractiveGrid->horizontalHeader()->setMaximumHeight(QWIDGETSIZE_MAX);
  ui->tableInteractiveGrid->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  ui->tableInteractiveGrid->setMouseTracking(true);
  ui->tableInteractiveGrid->viewport()->setMouseTracking(true);
  ui->tableInteractiveGrid->viewport()->installEventFilter(this);
  ui->tableInteractiveGrid->setMinimumHeight(0);
  ui->tableInteractiveGrid->setMaximumHeight(QWIDGETSIZE_MAX);
  ui->frameDangKyContainer->setMinimumHeight(0);
  ui->frameDangKyContainer->setMaximumHeight(QWIDGETSIZE_MAX);
  ui->frameDangKyContainer->setSizePolicy(QSizePolicy::Expanding,
                                          QSizePolicy::Expanding);
  ui->tableInteractiveGrid->viewport()->unsetCursor();

  buildInteractiveGrid(openTime, closeTime);

  // Update column headers with actual dates
  QStringList headers;
  for (int i = 0; i < 7; ++i)
  {
    QDate d = weekStart.addDays(i);
    int dow = d.dayOfWeek(); // 1=Mon .. 7=Sun
    QString dayName = (dow == 7) ? "Chủ Nhật" : QString("Thứ %1").arg(dow + 1);
    headers << QString("%1\n%2").arg(dayName, d.toString("dd-MM-yyyy"));
  }
  ui->tableInteractiveGrid->setHorizontalHeaderLabels(headers);
  updatePartTimeWeekMetadata(weekStart);
}

void Schedule_View::buildFullTimeGrid()
{
  QTableWidget *grid = ui->tableInteractiveGrid;
  grid->clearContents();
  grid->setRowCount(3);
  grid->setColumnCount(7);
  grid->setSelectionMode(QAbstractItemView::NoSelection);
  grid->setSelectionBehavior(QAbstractItemView::SelectItems);
  grid->setMouseTracking(true);
  grid->viewport()->setMouseTracking(true);
  grid->viewport()->installEventFilter(this);
  grid->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  grid->horizontalHeader()->setFixedHeight(56);
  grid->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
  grid->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  grid->verticalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  grid->horizontalHeader()->setStyleSheet(
      "QHeaderView::section { background-color:#1D4ED8;color:white;"
      "font-size:12px;font-weight:700;padding:7px;border:none;"
      "border-right:1px solid #3B82F6; }");
  grid->verticalHeader()->setStyleSheet(
      "QHeaderView::section {"
      "background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
      "stop:0 #2563EB,stop:0.035 #2563EB,stop:0.036 #F8FAFC,stop:1 #F8FAFC);"
      "color:#1E293B;font-size:12px;font-weight:700;"
      "padding:12px 14px;border:none;border-right:1px solid #CBD5E1;"
      "border-bottom:1px solid #E2E8F0; }");

  QStringList shiftLabels = {
      QString("%1\n%2").arg(SHIFT_NAMES[0], SHIFT_TIMES[0]),
      QString("%1\n%2").arg(SHIFT_NAMES[1], SHIFT_TIMES[1]),
      QString("%1\n%2").arg(SHIFT_NAMES[2], SHIFT_TIMES[2])};
  grid->setVerticalHeaderLabels(shiftLabels);
  grid->verticalHeader()->setVisible(true);
  grid->verticalHeader()->setMinimumWidth(165);
  grid->verticalHeader()->setMaximumWidth(165);

  for (int row = 0; row < 3; ++row)
  {
    grid->setRowHeight(row, 96);
    for (int col = 0; col < 7; ++col)
    {
      QTableWidgetItem *item = new QTableWidgetItem();
      item->setTextAlignment(Qt::AlignCenter);
      grid->setItem(row, col, item);
    }
  }

  grid->setMinimumHeight(440);
  grid->setMaximumHeight(560);
  grid->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  ui->frameDangKyContainer->setMinimumHeight(440);
  ui->frameDangKyContainer->setMaximumHeight(560);
  ui->frameDangKyContainer->setSizePolicy(QSizePolicy::Expanding,
                                          QSizePolicy::Expanding);
}

void Schedule_View::setUpFullTimeScheduleGrid(
    QDate weekStart, const FullTimeScheduleGrid &statuses)
{
  m_isFullTimeMode = true;
  m_partTimeDragActive = false;
  m_partTimeDragVisited.clear();
  if (partTimeInfoWidget)
    partTimeInfoWidget->setVisible(false);
  if (lblPartTimeFooterMessage)
    lblPartTimeFooterMessage->setVisible(false);
  m_fullTimeStatuses = FullTimeScheduleGrid(
      7, QList<FullTimeShiftStatus>(3, FullTimeShiftStatus::Available));

  for (int day = 0; day < statuses.size() && day < 7; ++day)
    for (int shift = 0; shift < statuses[day].size() && shift < 3; ++shift)
      m_fullTimeStatuses[day][shift] = statuses[day][shift];

  buildFullTimeGrid();

  QStringList headers;
  for (int day = 0; day < 7; ++day)
  {
    QDate date = weekStart.addDays(day);
    int dayOfWeek = date.dayOfWeek();
    QString dayName = (dayOfWeek == 7)
        ? "Chủ Nhật"
        : QString("Thứ %1").arg(dayOfWeek + 1);
    headers << QString("%1\n%2").arg(dayName, date.toString("dd-MM-yyyy"));
  }
  ui->tableInteractiveGrid->setHorizontalHeaderLabels(headers);

  m_fullTimeSelections.clear();
  for (int day = 0; day < 7; ++day)
  {
    for (int shift = 0; shift < 3; ++shift)
    {
      if (m_fullTimeStatuses[day][shift] == FullTimeShiftStatus::Pending)
        m_fullTimeSelections.insert(qMakePair(shift, day));
      renderFullTimeCell(shift, day);
    }
  }

  connect(ui->tableInteractiveGrid, &QTableWidget::cellClicked,
          this, &Schedule_View::onFullTimeCellClicked, Qt::UniqueConnection);

  ui->DangKyLich->setText("ĐĂNG KÝ CA TOÀN THỜI GIAN");
  ui->DangKyLich->setStyleSheet(
      "color:#1F2937;font-size:20px;font-weight:700;padding:2px 0 4px 0;");
  updateFullTimeWeekMetadata(weekStart);
  resetFullTimeFooterHint();
  fullTimeInfoWidget->setVisible(true);
}

void Schedule_View::updateFullTimeWeekMetadata(QDate weekStart)
{
  if (!lblFullTimeWeekRange)
    return;
  lblFullTimeWeekRange->setText(
      QString("Tuần đăng ký: %1 - %2")
          .arg(weekStart.toString("dd-MM-yyyy"),
               weekStart.addDays(6).toString("dd-MM-yyyy")));
}

void Schedule_View::resetFullTimeFooterHint()
{
  if (!lblFullTimeFooterMessage)
    return;
  lblFullTimeFooterMessage->setText(
      "Các ca màu đỏ hiện không thể đăng ký.");
  lblFullTimeFooterMessage->setStyleSheet(
      "color:#64748B;font-size:12px;padding-left:4px;");
  lblFullTimeFooterMessage->setVisible(true);
}

void Schedule_View::showFullTimeSaveFeedback(const QString &message)
{
  if (!lblFullTimeFooterMessage)
    return;
  lblFullTimeFooterMessage->setText(message);
  lblFullTimeFooterMessage->setStyleSheet(
      "color:#047857;background-color:#ECFDF5;border:1px solid #A7F3D0;"
      "border-radius:8px;padding:6px 10px;font-size:12px;font-weight:700;");
  lblFullTimeFooterMessage->setVisible(true);
}

void Schedule_View::renderFullTimeCell(int row, int col)
{
  if (row < 0 || row >= 3 || col < 0 || col >= 7)
    return;

  QTableWidgetItem *item = ui->tableInteractiveGrid->item(row, col);
  if (!item)
    return;

  FullTimeShiftStatus savedStatus = m_fullTimeStatuses[col][row];
  bool selected = m_fullTimeSelections.contains(qMakePair(row, col));

  QFont font = item->font();
  font.setBold(true);
  item->setFont(font);
  item->setToolTip(QString());

  if (savedStatus == FullTimeShiftStatus::StaffShortage)
  {
    item->setText("Thiếu nhân viên");
    item->setData(FULL_TIME_VISUAL_ROLE, VisualStaffShortage);
    item->setFlags(Qt::NoItemFlags);
    item->setToolTip("Ca này hiện không thể đăng ký cho vai trò này");
    return;
  }

  if (savedStatus == FullTimeShiftStatus::Approved)
  {
    item->setText("Đã duyệt");
    item->setData(FULL_TIME_VISUAL_ROLE, VisualApproved);
    item->setFlags(Qt::NoItemFlags);
    item->setToolTip("Ca đã duyệt và không thể thay đổi");
    return;
  }

  item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  if (savedStatus == FullTimeShiftStatus::Pending && !selected)
  {
    item->setText("Bỏ đăng ký");
    item->setData(FULL_TIME_VISUAL_ROLE, VisualPendingRemoval);
  }
  else if (savedStatus == FullTimeShiftStatus::Pending && selected)
  {
    item->setText("Chờ duyệt");
    item->setData(FULL_TIME_VISUAL_ROLE, VisualRegistered);
  }
  else if (selected)
  {
    item->setText("Đã chọn");
    item->setData(FULL_TIME_VISUAL_ROLE, VisualSelected);
  }
  else if (savedStatus == FullTimeShiftStatus::Declined)
  {
    item->setText("Đã từ chối\nNhấn để chọn lại");
    item->setData(FULL_TIME_VISUAL_ROLE, VisualDeclined);
    item->setToolTip("Ca từng bị từ chối; nhấn để đăng ký lại");
  }
  else
  {
    item->setText("Chưa đăng ký");
    item->setData(FULL_TIME_VISUAL_ROLE, VisualUnregistered);
    item->setToolTip("Nhấn để chọn");
    font.setBold(false);
    item->setFont(font);
  }
}

void Schedule_View::onFullTimeCellClicked(int row, int col)
{
  if (!m_isFullTimeMode || row < 0 || row >= 3 || col < 0 || col >= 7)
    return;
  if (m_fullTimeStatuses[col][row] == FullTimeShiftStatus::StaffShortage ||
      m_fullTimeStatuses[col][row] == FullTimeShiftStatus::Approved)
    return;

  QPair<int, int> coordinate = qMakePair(row, col);
  if (m_fullTimeSelections.contains(coordinate))
    m_fullTimeSelections.remove(coordinate);
  else
    m_fullTimeSelections.insert(coordinate);

  renderFullTimeCell(row, col);
  resetFullTimeFooterHint();
  ui->tableInteractiveGrid->viewport()->update();
}

bool Schedule_View::eventFilter(QObject *watched, QEvent *event)
{
  if (watched == ui->tableInteractiveGrid->viewport())
  {
    if (!m_isFullTimeMode && m_partTimeRegistrationOpen &&
        event->type() == QEvent::MouseButtonPress)
    {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
      if (mouseEvent->button() == Qt::LeftButton)
      {
        QModelIndex index = ui->tableInteractiveGrid->indexAt(
            mouseEvent->position().toPoint());
        QTableWidgetItem *item = index.isValid()
            ? ui->tableInteractiveGrid->item(index.row(), index.column())
            : nullptr;
        if (item && item->flags().testFlag(Qt::ItemIsSelectable))
        {
          m_partTimeDragActive = true;
          m_partTimeDragSelect = !item->isSelected();
          m_partTimeDragVisited.clear();
          m_partTimeDragVisited.insert(qMakePair(index.row(), index.column()));
          setPartTimeItemSelected(item, m_partTimeDragSelect);
          ui->tableInteractiveGrid->setCurrentItem(
              item, QItemSelectionModel::NoUpdate);
          ui->tableInteractiveGrid->viewport()->update();
          return true;
        }
      }
    }

    if (event->type() == QEvent::MouseMove)
    {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
      QModelIndex index = ui->tableInteractiveGrid->indexAt(
          mouseEvent->position().toPoint());

      if (!m_isFullTimeMode && m_partTimeDragActive &&
          mouseEvent->buttons().testFlag(Qt::LeftButton) && index.isValid())
      {
        QPair<int, int> coordinate = qMakePair(index.row(), index.column());
        if (!m_partTimeDragVisited.contains(coordinate))
        {
          QTableWidgetItem *item = ui->tableInteractiveGrid->item(
              index.row(), index.column());
          if (item && item->flags().testFlag(Qt::ItemIsSelectable))
          {
            setPartTimeItemSelected(item, m_partTimeDragSelect);
            m_partTimeDragVisited.insert(coordinate);
          }
        }
      }

      if (m_isFullTimeMode && index.isValid() &&
          index.column() < m_fullTimeStatuses.size() &&
          index.row() < m_fullTimeStatuses[index.column()].size())
      {
        FullTimeShiftStatus status =
            m_fullTimeStatuses[index.column()][index.row()];
        bool unavailable = status == FullTimeShiftStatus::StaffShortage ||
                           status == FullTimeShiftStatus::Approved;
        ui->tableInteractiveGrid->viewport()->setCursor(
            unavailable ? Qt::ForbiddenCursor : Qt::PointingHandCursor);
      }
      else if (!m_isFullTimeMode && index.isValid())
      {
        QTableWidgetItem *item = ui->tableInteractiveGrid->item(
            index.row(), index.column());
        bool selectable = m_partTimeRegistrationOpen && item &&
                          item->flags().testFlag(Qt::ItemIsSelectable);
        ui->tableInteractiveGrid->viewport()->setCursor(
            selectable ? Qt::PointingHandCursor : Qt::ForbiddenCursor);
      }
      else
      {
        ui->tableInteractiveGrid->viewport()->setCursor(
            (!m_isFullTimeMode && !m_partTimeRegistrationOpen)
                ? Qt::ForbiddenCursor
                : Qt::ArrowCursor);
      }
      ui->tableInteractiveGrid->viewport()->update();
      if (!m_isFullTimeMode && m_partTimeDragActive)
        return true;
    }
    else if (!m_isFullTimeMode &&
             event->type() == QEvent::MouseButtonRelease)
    {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
      if (mouseEvent->button() == Qt::LeftButton && m_partTimeDragActive)
      {
        m_partTimeDragActive = false;
        m_partTimeDragVisited.clear();
        ui->tableInteractiveGrid->viewport()->update();
        return true;
      }
    }
    else if (event->type() == QEvent::Leave)
    {
      m_partTimeDragActive = false;
      m_partTimeDragVisited.clear();
      ui->tableInteractiveGrid->viewport()->setCursor(
          (!m_isFullTimeMode && !m_partTimeRegistrationOpen)
              ? Qt::ForbiddenCursor
              : Qt::ArrowCursor);
      ui->tableInteractiveGrid->viewport()->update();
    }
  }

  return QWidget::eventFilter(watched, event);
}

void Schedule_View::updateStaffInteractiveGridStatus(
    const QMap<int, QList<Shift*>> &pendingShifts,
    const QMap<int, QList<Shift*>> &acceptedShifts,
    const QMap<int, QMap<int, ShiftBlock *>> &managerGrid)
{
  QTableWidget *grid = ui->tableInteractiveGrid;
  int rowCount = grid->rowCount();
  int openHour = m_openHour;
  grid->clearSelection();
  m_partTimePendingCells.clear();
  m_partTimeApprovedDays.clear();

  for (int col = 0; col < 7; ++col)
  {
    for (int row = 0; row < rowCount; ++row)
    {
      int currentHour = openHour + row;
      QTableWidgetItem *item = grid->item(row, col);
      if (!item)
        continue;

      PartTimeVisualState visualState = HourlyAvailable;
      QString tooltip = "Chưa có dữ liệu nhân sự";

      bool accepted = false;
      if (acceptedShifts.contains(col))
      {
        for (Shift *shift : acceptedShifts[col])
        {
          if (shift->getStartTime().hour() <= currentHour &&
              shift->getEndTime().hour() > currentHour)
          {
            accepted = true;
            break;
          }
        }
      }

      bool pending = false;
      if (!accepted && pendingShifts.contains(col))
      {
        for (Shift *shift : pendingShifts[col])
        {
          if (shift->getStartTime().hour() <= currentHour &&
              shift->getEndTime().hour() > currentHour)
          {
            pending = true;
            break;
          }
        }
      }

      if (accepted)
      {
        visualState = HourlyApproved;
        tooltip = "Đã được duyệt";
      }
      else if (pending)
      {
        visualState = HourlyPending;
        tooltip = "Đang chờ duyệt";
      }
      else
      {
        int shiftRow = -1;
        if (currentHour >= 7 && currentHour < 12)
          shiftRow = 0;
        else if (currentHour >= 12 && currentHour < 17)
          shiftRow = 1;
        else if (currentHour >= 17 && currentHour < 22)
          shiftRow = 2;

        if (shiftRow >= 0 && managerGrid.contains(col) &&
            managerGrid[col].contains(shiftRow))
        {
          ShiftBlock *block = managerGrid[col][shiftRow];
          if (block && (block->getStatus() == ShiftStatus::Empty ||
                        block->getStatus() == ShiftStatus::Understaffed))
          {
            visualState = HourlyUnderstaffed;
            tooltip = "Thiếu nhân viên";
          }
          else if (block)
          {
            visualState = HourlySufficient;
            tooltip = "Đã đủ nhân viên";
          }
        }
      }

      item->setText(QString());
      item->setToolTip(tooltip);
      item->setData(PART_TIME_VISUAL_ROLE, visualState);
      item->setData(PART_TIME_MUTED_ROLE, !m_partTimeRegistrationOpen);
      item->setData(SHIFT_BOUNDARY_ROLE,
                    row == 0 || row == 5 || row == 10);
      bool selectable = m_partTimeRegistrationOpen && !accepted;
      item->setFlags(selectable
                         ? Qt::ItemIsEnabled | Qt::ItemIsSelectable
                         : Qt::NoItemFlags);
      if (pending)
        m_partTimePendingCells.insert(qMakePair(row, col));
      if (accepted)
        m_partTimeApprovedDays.insert(col);
      setPartTimeItemSelected(item, pending && selectable);
    }
  }

  grid->viewport()->update();
}

void Schedule_View::setPartTimeItemSelected(QTableWidgetItem *item,
                                            bool selected)
{
  if (!item)
    return;

  item->setSelected(selected);
  PartTimeVisualState visualState = static_cast<PartTimeVisualState>(
      item->data(PART_TIME_VISUAL_ROLE).toInt());
  if (visualState == HourlyPending)
  {
    item->setToolTip(
        selected
            ? "Đang chờ duyệt. Kéo lại để bỏ đăng ký."
            : "Sẽ hủy đăng ký đang chờ duyệt khi lưu.");
  }
}

void Schedule_View::updatePartTimeWeekMetadata(QDate weekStart)
{
  if (!lblPartTimeWeekRange)
    return;
  lblPartTimeWeekRange->setText(
      QString("Tuần đăng ký: %1 - %2")
          .arg(weekStart.toString("dd-MM-yyyy"),
               weekStart.addDays(6).toString("dd-MM-yyyy")));
}

void Schedule_View::updatePartTimeInfoText()
{
  if (!lblPartTimeRegistrationState || !lblPartTimeFooterMessage)
    return;

  if (m_partTimeRegistrationOpen)
  {
    lblPartTimeRegistrationState->setText(
        "Chọn các khung giờ bạn có thể làm việc.");
    lblPartTimeRegistrationState->setStyleSheet(
        "color:#475569;font-size:12px;");
    lblPartTimeFooterMessage->setText(
        "Chọn tối thiểu số ngày được yêu cầu trước khi lưu.");
  }
  else
  {
    int dayOfWeek = m_partTimeNextOpenDate.dayOfWeek();
    QString dayName = (dayOfWeek == 7)
        ? "Chủ Nhật"
        : QString("Thứ %1").arg(dayOfWeek + 1);
    lblPartTimeRegistrationState->setText(
        QString("Đăng ký ca chưa mở - mở vào %1, %2.")
            .arg(dayName, m_partTimeNextOpenDate.toString("dd-MM-yyyy")));
    lblPartTimeRegistrationState->setStyleSheet(
        "color:#92400E;font-size:12px;font-weight:600;");
    lblPartTimeFooterMessage->setText(
        "Bạn có thể lưu lịch khi đợt đăng ký mở.");
  }

  lblPartTimeFooterMessage->setStyleSheet(
      "color:#64748B;font-size:12px;padding-left:4px;");
  lblPartTimeFooterMessage->setVisible(true);
}

void Schedule_View::setPartTimeRegistrationState(bool isOpen,
                                                 QDate nextOpenDate)
{
  m_partTimeRegistrationOpen = isOpen;
  m_partTimeDragActive = false;
  m_partTimeDragVisited.clear();
  m_partTimeNextOpenDate = nextOpenDate;
  ui->tableInteractiveGrid->setEnabled(isOpen);
  ui->buttonLuu->setEnabled(isOpen);
  ui->DangKyLich->setText("ĐĂNG KÝ LỊCH LÀM");
  ui->DangKyLich->setStyleSheet(
      "color:#1F2937;font-size:20px;font-weight:700;padding:2px 0 4px 0;");
  ui->tableInteractiveGrid->viewport()->setCursor(
      isOpen ? Qt::ArrowCursor : Qt::ForbiddenCursor);
  updatePartTimeInfoText();
}

void Schedule_View::enableRegistration(bool isEnable)
{
  ui->tableInteractiveGrid->setEnabled(isEnable);
  ui->buttonLuu->setEnabled(isEnable);
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
  if (m_isFullTimeMode)
  {
    QList<QList<int>> selectedByDay(7);
    for (const QPair<int, int> &coordinate : m_fullTimeSelections)
      selectedByDay[coordinate.second].append(coordinate.first);

    for (QList<int> &selectedRows : selectedByDay)
      std::sort(selectedRows.begin(), selectedRows.end());

    emit requestSaveFullTimeSchedule(selectedByDay);
    return;
  }

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
  int minDays = Config::getMinimumDaysWorkPerWeek_PT();
  QSet<int> registeredDays = daysWithSelection;
  registeredDays.unite(m_partTimeApprovedDays);
  bool removingAllPending = daysWithSelection.isEmpty() &&
                            !m_partTimePendingCells.isEmpty();
  if (static_cast<int>(registeredDays.size()) < minDays &&
      !removingAllPending)
  {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Chưa đủ số ngày đăng ký");
    msgBox.setText(
        QString("Bạn cần đăng ký tối thiểu %1 ngày trong tuần.\n"
                "Hiện tại bạn chỉ chọn %2 ngày.")
            .arg(minDays)
            .arg(registeredDays.size()));
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
