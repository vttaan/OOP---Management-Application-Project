#include "Schedule_View.h"
#include "global.h"
#include "ui_Schedule_View.h"
#include "utils/Config.h"
// ─── Shift label constants
// ────────────────────────────────────────────────────
static const QString SHIFT_NAMES[3] = {"Ca Sáng", "Ca Chiều", "Ca Tối"};
static const QString SHIFT_TIMES[3] = {"07:00 - 12:00", "12:00 - 17:00",
                                       "17:00 - 22:00"};

// Selected cell highlight colour (staff interactive grid)
static const QColor SELECTED_COLOR(0xBF, 0xDB, 0xFE);   // light blue
static const QColor SELECTED_BORDER(0x1D, 0x4E, 0xD8);  // blue-700

namespace {
constexpr int FULL_TIME_VISUAL_ROLE = Qt::UserRole + 1;
constexpr int PART_TIME_VISUAL_ROLE = Qt::UserRole + 2;
constexpr int PART_TIME_MUTED_ROLE = Qt::UserRole + 3;
constexpr int SHIFT_BOUNDARY_ROLE = Qt::UserRole + 4;

enum FullTimeVisualState {
  VisualUnregistered = 0,
  VisualRegistered,
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

class ScheduleGridDelegate final : public QStyledItemDelegate
{
public:
  explicit ScheduleGridDelegate(QObject *parent = nullptr)
      : QStyledItemDelegate(parent) {}

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override
  {
    QVariant partTimeStateData = index.data(PART_TIME_VISUAL_ROLE);
    if (partTimeStateData.isValid())
    {
      paintPartTimeCell(painter, option, index,
                        static_cast<PartTimeVisualState>(partTimeStateData.toInt()));
      return;
    }

    QVariant visualStateData = index.data(FULL_TIME_VISUAL_ROLE);
    if (!visualStateData.isValid())
    {
      QStyledItemDelegate::paint(painter, option, index);
      return;
    }

    FullTimeVisualState visualState =
        static_cast<FullTimeVisualState>(visualStateData.toInt());
    QColor background;
    QColor border;
    QColor foreground;
    bool bold = true;

    switch (visualState)
    {
      case VisualRegistered:
        background = QColor("#ECFDF5");
        border = QColor("#A7F3D0");
        foreground = QColor("#047857");
        break;
      case VisualSelected:
        background = QColor("#EFF6FF");
        border = QColor("#3B82F6");
        foreground = QColor("#1D4ED8");
        break;
      case VisualPendingRemoval:
        background = QColor("#FFFBEB");
        border = QColor("#FDE68A");
        foreground = QColor("#92400E");
        break;
      case VisualStaffShortage:
        background = QColor("#FEF2F2");
        border = QColor("#FECACA");
        foreground = QColor("#DC2626");
        break;
      case VisualUnregistered:
      default:
        background = QColor("#F8FAFC");
        border = QColor("#E2E8F0");
        foreground = QColor("#64748B");
        bold = false;
        break;
    }

    bool canHover = visualState != VisualStaffShortage;
    if (canHover && option.state.testFlag(QStyle::State_MouseOver))
    {
      background = QColor("#EFF6FF");
      border = QColor("#60A5FA");
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->fillRect(option.rect, QColor("#FFFFFF"));

    painter->setPen(QPen(QColor("#E5E7EB"), 1));
    painter->drawRect(option.rect.adjusted(0, 0, -1, -1));

    QRectF cardRect = QRectF(option.rect).adjusted(14, 24, -14, -24);
    painter->setBrush(background);
    painter->setPen(QPen(border, visualState == VisualSelected ? 2.0 : 1.0));
    painter->drawRoundedRect(cardRect, 10, 10);

    QFont font = option.font;
    font.setBold(bold);
    font.setPointSizeF(qMax(9.0, font.pointSizeF()));
    painter->setFont(font);
    painter->setPen(foreground);
    painter->drawText(cardRect.adjusted(8, 4, -8, -4),
                      Qt::AlignCenter | Qt::TextWordWrap,
                      index.data(Qt::DisplayRole).toString());
    painter->restore();
  }

private:
  void paintPartTimeCell(QPainter *painter,
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index,
                         PartTimeVisualState visualState) const
  {
    bool muted = index.data(PART_TIME_MUTED_ROLE).toBool();
    bool shiftBoundary = index.data(SHIFT_BOUNDARY_ROLE).toBool();
    bool selected = option.state.testFlag(QStyle::State_Selected);
    bool hovered = option.state.testFlag(QStyle::State_MouseOver) && !muted;
    bool pendingRemoval = visualState == HourlyPending && !selected && !muted;

    QColor background;
    QColor border;
    switch (visualState)
    {
      case HourlyUnderstaffed:
        background = muted ? QColor("#FFF7F7") : QColor("#FEF2F2");
        border = muted ? QColor("#F3DEDE") : QColor("#FECACA");
        break;
      case HourlySufficient:
        background = muted ? QColor("#F5FBF7") : QColor("#ECFDF5");
        border = muted ? QColor("#DCECE2") : QColor("#A7F3D0");
        break;
      case HourlyPending:
        background = muted ? QColor("#FFFCF2") : QColor("#FEF9C3");
        border = muted ? QColor("#EEE8D3") : QColor("#FDE68A");
        break;
      case HourlyApproved:
        background = muted ? QColor("#F2FAF5") : QColor("#D1FAE5");
        border = muted ? QColor("#D6E9DC") : QColor("#6EE7B7");
        break;
      case HourlyAvailable:
      default:
        background = muted ? QColor("#F8FAFC") : QColor("#FFFFFF");
        border = QColor("#E2E8F0");
        break;
    }

    if (pendingRemoval)
    {
      background = QColor("#FFF7ED");
      border = QColor("#FDBA74");
    }
    else if (hovered)
    {
      background = QColor("#EFF6FF");
      border = QColor("#60A5FA");
    }
    if (selected && visualState != HourlyPending)
    {
      background = QColor("#DBEAFE");
      border = QColor("#2563EB");
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->fillRect(option.rect, QColor("#FFFFFF"));

    QRectF cellRect = QRectF(option.rect).adjusted(4, 3, -4, -3);
    painter->setBrush(background);
    painter->setPen(QPen(border, selected ? 2.0 : 1.0));
    painter->drawRoundedRect(cellRect, 6, 6);

    if (pendingRemoval)
    {
      QFont font = option.font;
      font.setBold(true);
      font.setPointSizeF(qMax(8.0, font.pointSizeF() - 1.0));
      painter->setFont(font);
      painter->setPen(QColor("#C2410C"));
      painter->drawText(cellRect.adjusted(4, 2, -4, -2),
                        Qt::AlignCenter, "Bỏ đăng ký");
    }

    if (shiftBoundary)
    {
      painter->setPen(QPen(QColor("#3B82F6"), 2));
      painter->drawLine(option.rect.topLeft(), option.rect.topRight());
    }
    painter->restore();
  }
};

QLabel *makeLegendPill(const QString &text, const QString &background,
                       const QString &foreground, QWidget *parent)
{
  QLabel *pill = new QLabel(text, parent);
  pill->setAlignment(Qt::AlignCenter);
  pill->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  pill->setFixedHeight(30);
  pill->setStyleSheet(
      QString("background-color:%1;color:%2;border-radius:9px;"
              "padding:4px 10px;font-size:11px;font-weight:600;")
          .arg(background, foreground));
  return pill;
}
}

Schedule_View::Schedule_View(QWidget *parent)
    : QWidget(parent), ui(new Ui::Schedule_View), missingStaffWidget(nullptr),
      lblMissingStaffHeader(nullptr), lblMissingCount(nullptr),
      tableMissingStaff(nullptr), fullTimeInfoWidget(nullptr),
      lblFullTimeWeekRange(nullptr), lblFullTimeFooterMessage(nullptr),
      partTimeInfoWidget(nullptr), lblPartTimeRegistrationState(nullptr),
      lblPartTimeWeekRange(nullptr), lblPartTimeFooterMessage(nullptr)
{
  ui->setupUi(this);
  setUpUI();
  connect(ui->btnGenSchedule, &QPushButton::clicked, this,
          &Schedule_View::requestGenSchedule);
  connect(ui->btnConfirm, &QPushButton::clicked, this,
          &Schedule_View::requestConfirm);
  connect(ui->buttonLuu, &QPushButton::clicked, this,
          &Schedule_View::buttonSaveClicked);
}

Schedule_View::~Schedule_View() { delete ui; }

void Schedule_View::setUpUI()
{
  ui->verticalLayout->setContentsMargins(18, 14, 18, 14);
  ui->verticalLayout->setSpacing(10);
  ui->verticalLayout->setAlignment(Qt::AlignTop);
  ui->DangKyLich->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  ui->DangKyLich->setFixedHeight(48);
  ui->DangKyLich->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

  // ── Interactive grid (staff mode) ─────────────────────────────────────────
  ui->tableInteractiveGrid->setSelectionMode(QAbstractItemView::MultiSelection);
  ui->tableInteractiveGrid->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->tableInteractiveGrid->setFocusPolicy(Qt::StrongFocus);
  ui->tableInteractiveGrid->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->tableInteractiveGrid->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->tableInteractiveGrid->verticalHeader()->setMinimumWidth(110);
  ui->tableInteractiveGrid->setSelectionBehavior(QAbstractItemView::SelectItems);
  ui->tableInteractiveGrid->setItemDelegate(
      new ScheduleGridDelegate(ui->tableInteractiveGrid));

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
  ui->DangKyLich->setStyleSheet(
      "color:#1F2937;font-size:20px;font-weight:700;padding:2px 0 4px 0;");

  ui->buttonLuu->setStyleSheet(
      "QPushButton { background-color: #219653; color: white; border-radius: "
      "8px; padding: 10px 32px; font-weight: bold; font-size: 14px; } "
      "QPushButton:hover { background-color: #1E824C; }"
      "QPushButton:pressed { background-color: #166534; }"
      "QPushButton:disabled { background-color: #D1D5DB; color: #6B7280; }");

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

  fullTimeInfoWidget = new QFrame(this);
  fullTimeInfoWidget->setObjectName("fullTimeInfoWidget");
  fullTimeInfoWidget->setStyleSheet(
      "QFrame#fullTimeInfoWidget { background-color:#F8FAFC;"
      "border:1px solid #E2E8F0;border-radius:10px; }"
      "QLabel { background-color:transparent;border:none; }");
  fullTimeInfoWidget->setSizePolicy(QSizePolicy::Expanding,
                                    QSizePolicy::Fixed);
  fullTimeInfoWidget->setFixedHeight(92);
  fullTimeInfoWidget->setVisible(false);

  QVBoxLayout *infoLayout = new QVBoxLayout(fullTimeInfoWidget);
  infoLayout->setContentsMargins(14, 10, 14, 10);
  infoLayout->setSpacing(8);

  QHBoxLayout *descriptionRow = new QHBoxLayout();
  QLabel *subtitle = new QLabel(
      "Chọn một hoặc nhiều ca trong tuần để đăng ký.",
      fullTimeInfoWidget);
  subtitle->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  subtitle->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  subtitle->setFixedHeight(20);
  subtitle->setStyleSheet("color:#475569;font-size:12px;");
  lblFullTimeWeekRange = new QLabel(fullTimeInfoWidget);
  lblFullTimeWeekRange->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  lblFullTimeWeekRange->setSizePolicy(QSizePolicy::Preferred,
                                      QSizePolicy::Fixed);
  lblFullTimeWeekRange->setFixedHeight(20);
  lblFullTimeWeekRange->setStyleSheet(
      "color:#1E40AF;font-size:12px;font-weight:700;");
  descriptionRow->addWidget(subtitle);
  descriptionRow->addStretch();
  descriptionRow->addWidget(lblFullTimeWeekRange);
  infoLayout->addLayout(descriptionRow);

  QHBoxLayout *legendRow = new QHBoxLayout();
  legendRow->setSpacing(8);
  QLabel *legendTitle = new QLabel("Trạng thái:", fullTimeInfoWidget);
  legendTitle->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  legendTitle->setFixedHeight(30);
  legendTitle->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  legendTitle->setStyleSheet("color:#64748B;font-size:11px;font-weight:600;");
  legendRow->addWidget(legendTitle);
  legendRow->addWidget(makeLegendPill("Đã đăng ký", "#D1FAE5", "#047857",
                                      fullTimeInfoWidget));
  legendRow->addWidget(makeLegendPill("Chưa đăng ký", "#E2E8F0", "#475569",
                                      fullTimeInfoWidget));
  legendRow->addWidget(makeLegendPill("Thiếu nhân viên", "#FEE2E2", "#DC2626",
                                      fullTimeInfoWidget));
  legendRow->addStretch();
  infoLayout->addLayout(legendRow);
  ui->verticalLayout->insertWidget(1, fullTimeInfoWidget);

  lblFullTimeFooterMessage = new QLabel(this);
  lblFullTimeFooterMessage->setVisible(false);
  ui->horizontalLayout_2->insertWidget(0, lblFullTimeFooterMessage);

  partTimeInfoWidget = new QFrame(this);
  partTimeInfoWidget->setObjectName("partTimeInfoWidget");
  partTimeInfoWidget->setStyleSheet(
      "QFrame#partTimeInfoWidget { background-color:#F8FAFC;"
      "border:1px solid #E2E8F0;border-radius:10px; }"
      "QLabel { background-color:transparent;border:none; }");
  partTimeInfoWidget->setSizePolicy(QSizePolicy::Expanding,
                                    QSizePolicy::Fixed);
  partTimeInfoWidget->setFixedHeight(92);
  partTimeInfoWidget->setVisible(false);

  QVBoxLayout *partTimeInfoLayout = new QVBoxLayout(partTimeInfoWidget);
  partTimeInfoLayout->setContentsMargins(14, 10, 14, 10);
  partTimeInfoLayout->setSpacing(8);

  QHBoxLayout *partTimeStateRow = new QHBoxLayout();
  lblPartTimeRegistrationState = new QLabel(partTimeInfoWidget);
  lblPartTimeRegistrationState->setSizePolicy(QSizePolicy::Preferred,
                                               QSizePolicy::Fixed);
  lblPartTimeRegistrationState->setFixedHeight(20);
  lblPartTimeRegistrationState->setStyleSheet(
      "color:#475569;font-size:12px;");
  lblPartTimeWeekRange = new QLabel(partTimeInfoWidget);
  lblPartTimeWeekRange->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  lblPartTimeWeekRange->setSizePolicy(QSizePolicy::Preferred,
                                      QSizePolicy::Fixed);
  lblPartTimeWeekRange->setFixedHeight(20);
  lblPartTimeWeekRange->setStyleSheet(
      "color:#1E40AF;font-size:12px;font-weight:700;");
  partTimeStateRow->addWidget(lblPartTimeRegistrationState);
  partTimeStateRow->addStretch();
  partTimeStateRow->addWidget(lblPartTimeWeekRange);
  partTimeInfoLayout->addLayout(partTimeStateRow);

  QHBoxLayout *partTimeLegendRow = new QHBoxLayout();
  partTimeLegendRow->setSpacing(8);
  QLabel *partTimeLegendTitle = new QLabel("Trạng thái:", partTimeInfoWidget);
  partTimeLegendTitle->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  partTimeLegendTitle->setFixedHeight(30);
  partTimeLegendTitle->setStyleSheet(
      "color:#64748B;font-size:11px;font-weight:600;");
  partTimeLegendRow->addWidget(partTimeLegendTitle);
  partTimeLegendRow->addWidget(
      makeLegendPill("Thiếu", "#FEE2E2", "#B91C1C", partTimeInfoWidget));
  partTimeLegendRow->addWidget(
      makeLegendPill("Đủ", "#D1FAE5", "#047857", partTimeInfoWidget));
  partTimeLegendRow->addWidget(
      makeLegendPill("Chờ duyệt", "#FEF9C3", "#854D0E", partTimeInfoWidget));
  partTimeLegendRow->addWidget(
      makeLegendPill("Đã duyệt", "#A7F3D0", "#065F46", partTimeInfoWidget));
  partTimeLegendRow->addStretch();
  partTimeInfoLayout->addLayout(partTimeLegendRow);
  ui->verticalLayout->insertWidget(1, partTimeInfoWidget);

  lblPartTimeFooterMessage = new QLabel(this);
  lblPartTimeFooterMessage->setStyleSheet(
      "color:#64748B;font-size:12px;padding-left:4px;");
  lblPartTimeFooterMessage->setVisible(false);
  ui->horizontalLayout_2->insertWidget(0, lblPartTimeFooterMessage);

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

void Schedule_View::setUpFullTimeGrid(
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
      7, QList<FullTimeShiftStatus>(3, FullTimeShiftStatus::Unregistered));

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
      if (m_fullTimeStatuses[day][shift] == FullTimeShiftStatus::Registered)
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

  item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  if (savedStatus == FullTimeShiftStatus::Registered && !selected)
  {
    item->setText("Bỏ đăng ký");
    item->setData(FULL_TIME_VISUAL_ROLE, VisualPendingRemoval);
  }
  else if (savedStatus == FullTimeShiftStatus::Unregistered && selected)
  {
    item->setText("Đã chọn");
    item->setData(FULL_TIME_VISUAL_ROLE, VisualSelected);
  }
  else if (selected)
  {
    item->setText("Đã đăng ký");
    item->setData(FULL_TIME_VISUAL_ROLE, VisualRegistered);
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
  if (m_fullTimeStatuses[col][row] == FullTimeShiftStatus::StaffShortage)
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
        bool unavailable = m_fullTimeStatuses[index.column()][index.row()] ==
                           FullTimeShiftStatus::StaffShortage;
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

    emit requestSaveFullTimeShifts(selectedByDay);
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
