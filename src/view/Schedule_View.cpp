#include "Schedule_View.h"
#include "global.h"
#include "ui_Schedule_View.h"
#include "utils/Config.h"
#include <QProgressBar>
#include <QScrollArea>
namespace {
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
  VisualStaffShortage,
  VisualStaffSufficient
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
        background = QColor("#FEF9C3");
        border = QColor("#FDE68A");
        foreground = QColor("#854D0E");
        break;
      case VisualApproved:
        background = QColor("#D1FAE5");
        border = QColor("#6EE7B7");
        foreground = QColor("#065F46");
        break;
      case VisualDeclined:
        background = QColor("#FFF7ED");
        border = QColor("#FDBA74");
        foreground = QColor("#C2410C");
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
      case VisualStaffSufficient:
        background = QColor("#ECFDF5");
        border = QColor("#A7F3D0");
        foreground = QColor("#047857");
        break;
      case VisualUnregistered:
      default:
        background = QColor("#F8FAFC");
        border = QColor("#E2E8F0");
        foreground = QColor("#64748B");
        bold = false;
        break;
    }

    bool canHover = visualState != VisualApproved;
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
      tableMissingStaff(nullptr), lblManagerDraftStatus(nullptr),
      lblManagerSummary(nullptr), fullTimeInfoWidget(nullptr),
      lblFullTimeWeekRange(nullptr), lblFullTimeFooterMessage(nullptr),
      staffInfoStack(nullptr),
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

  QWidget *managerToolbar = new QWidget(this);
  managerToolbar->setObjectName("managerToolbar");
  auto *toolbarLayout = new QHBoxLayout(managerToolbar);
  toolbarLayout->setContentsMargins(0, 0, 0, 0);
  toolbarLayout->setSpacing(8);
  QPushButton *prevWeek = new QPushButton("<", managerToolbar);
  QPushButton *nextWeek = new QPushButton(">", managerToolbar);
  QPushButton *currentWeek = new QPushButton("Tuần hiện tại", managerToolbar);
  lblManagerWeek = new QLabel("Tuần xác nhận", managerToolbar);
  lblManagerWeek->setAlignment(Qt::AlignCenter);
  lblManagerWeek->setStyleSheet("font-weight:700;color:#1E3A8A;padding:8px 14px;");
  managerStatusFilter = new QComboBox(managerToolbar);
  managerStatusFilter->addItems({"Tất cả trạng thái", "Thiếu nhân sự", "Chờ duyệt", "Đã đủ"});
  managerRoleFilter = new QComboBox(managerToolbar);
  managerRoleFilter->addItems({"Tất cả vai trò", "Quản lý", "Thu ngân", "Nhân viên sảnh", "Phụ bếp"});
  managerUndoDraftButton = new QPushButton("↶ Hoàn tác", managerToolbar);
  managerClearDraftButton = new QPushButton("Xóa bản nháp", managerToolbar);
  const QString draftToolStyle =
      "QPushButton { background:#FFFFFF;color:#475569;border:1px solid #CBD5E1;"
      "border-radius:6px;padding:6px 10px;font-weight:600; }"
      "QPushButton:hover { background:#F8FAFC;border-color:#94A3B8; }"
      "QPushButton:disabled { color:#CBD5E1;border-color:#E2E8F0; }";
  managerUndoDraftButton->setStyleSheet(draftToolStyle);
  managerClearDraftButton->setStyleSheet(draftToolStyle);
  managerUndoDraftButton->setEnabled(false);
  managerClearDraftButton->setEnabled(false);
  toolbarLayout->addWidget(prevWeek);
  toolbarLayout->addWidget(lblManagerWeek, 1);
  toolbarLayout->addWidget(nextWeek);
  toolbarLayout->addWidget(currentWeek);
  toolbarLayout->addWidget(managerStatusFilter);
  toolbarLayout->addWidget(managerRoleFilter);
  toolbarLayout->addWidget(managerUndoDraftButton);
  toolbarLayout->addWidget(managerClearDraftButton);
  ui->verticalLayout->insertWidget(1, managerToolbar);
  managerToolbar->setVisible(false);
  connect(prevWeek, &QPushButton::clicked, this, &Schedule_View::requestPreviousManagerWeek);
  connect(nextWeek, &QPushButton::clicked, this, &Schedule_View::requestNextManagerWeek);
  connect(currentWeek, &QPushButton::clicked, this, &Schedule_View::requestCurrentManagerWeek);
  connect(managerUndoDraftButton, &QPushButton::clicked,
          this, &Schedule_View::requestUndoManagerDraft);
  connect(managerClearDraftButton, &QPushButton::clicked,
          this, &Schedule_View::requestClearManagerDraft);
  connect(managerStatusFilter, qOverload<int>(&QComboBox::currentIndexChanged), this,
          [this](int) { if (!m_lastAssignCounts.isEmpty()) updateAssignGrid(m_lastAssignCounts); });
  connect(managerRoleFilter, qOverload<int>(&QComboBox::currentIndexChanged), this,
          [this](int) {
            if (shiftDetailDrawer && shiftDetailDrawer->isVisible())
              showShiftRequestsDialog(m_lastDrawerRequests, m_lastDrawerShiftLabel,
                                      m_lastDrawerEligible, m_lastDrawerDate,
                                      m_lastDrawerStart, m_lastDrawerEnd);
          });

  QWidget *managerSummaryBar = new QWidget(this);
  managerSummaryBar->setObjectName("managerSummaryBar");
  auto *summaryLayout = new QHBoxLayout(managerSummaryBar);
  summaryLayout->setContentsMargins(0, 0, 0, 0);
  summaryLayout->setSpacing(8);
  auto makeMetric = [managerSummaryBar](const QString &title, const QString &value,
                                         const QString &bg, const QString &fg,
                                         QLabel **valueLabel) {
    QFrame *card = new QFrame(managerSummaryBar);
    card->setStyleSheet(QString("QFrame { background:%1;border:1px solid %2;border-radius:10px;padding:8px; }")
                        .arg(bg, bg));
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(12, 8, 12, 8);
    QLabel *t = new QLabel(title, card);
    t->setStyleSheet("color:#64748B;font-size:11px;font-weight:600;");
    QLabel *v = new QLabel(value, card);
    v->setStyleSheet(QString("color:%1;font-size:20px;font-weight:800;").arg(fg));
    layout->addWidget(t);
    layout->addWidget(v);
    *valueLabel = v;
    return card;
  };
  summaryLayout->addWidget(makeMetric("TỔNG SỐ CA", "0", "#EFF6FF", "#1D4ED8", &lblManagerTotal));
  summaryLayout->addWidget(makeMetric("CA CHƯA ĐỦ", "0", "#FEF2F2", "#B91C1C", &lblManagerShortage));
  summaryLayout->addWidget(makeMetric("YÊU CẦU CHỜ DUYỆT", "0", "#FFFBEB", "#B45309", &lblManagerPending));
  summaryLayout->addWidget(makeMetric("THAY ĐỔI CHƯA LƯU", "0", "#F3E8FF", "#7E22CE", &lblManagerDraft));
  lblManagerSummary = new QLabel(managerSummaryBar);
  lblManagerSummary->setVisible(false);
  lblManagerDraftStatus = new QLabel(managerSummaryBar);
  lblManagerDraftStatus->setVisible(false);
  ui->verticalLayout->insertWidget(2, managerSummaryBar);
  managerSummaryBar->setVisible(false);

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
  this->setStyleSheet("#ScheduleViewMain { background-color: #F8FAFC; color: #1F2937; } #ScheduleViewMain QLabel { background-color: transparent; }");
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

  // Manager workspace: weekly grid plus an embedded, non-blocking detail drawer.
  // Keep the weekly context at the same height whether the drawer is hidden,
  // empty, or contains many request/employee cards. Drawer content scrolls
  // internally instead of contributing its size hint to the shared workspace.
  constexpr int managerScheduleHeight = 450;
  ui->frameTableContainer->setFixedHeight(managerScheduleHeight);
  QWidget *managerWorkspace = new QWidget(ui->frameTableContainer);
  managerWorkspace->setFixedHeight(managerScheduleHeight);
  managerWorkspace->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QHBoxLayout *managerWorkspaceLayout = new QHBoxLayout(managerWorkspace);
  managerWorkspaceLayout->setContentsMargins(0, 0, 0, 0);
  managerWorkspaceLayout->setSpacing(12);
  ui->frameLayout->removeWidget(ui->tableSum);
  ui->tableSum->setFixedHeight(managerScheduleHeight);
  ui->tableSum->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  managerWorkspaceLayout->addWidget(ui->tableSum, 1);

  shiftDetailDrawer = new QFrame(managerWorkspace);
  shiftDetailDrawer->setObjectName("shiftDetailDrawer");
  shiftDetailDrawer->setMinimumWidth(380);
  shiftDetailDrawer->setMaximumWidth(440);
  shiftDetailDrawer->setFixedHeight(managerScheduleHeight);
  shiftDetailDrawer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  shiftDetailDrawer->setStyleSheet(
      "QFrame#shiftDetailDrawer { background:#FFFFFF;border:1px solid #D7E0EA;"
      "border-radius:14px; }"
      "QLabel { background:transparent;border:none; }");
  shiftDetailDrawerLayout = new QVBoxLayout(shiftDetailDrawer);
  shiftDetailDrawerLayout->setContentsMargins(18, 18, 18, 16);
  shiftDetailDrawerLayout->setSpacing(14);
  shiftDetailDrawer->setVisible(false);
  managerWorkspaceLayout->addWidget(shiftDetailDrawer);
  ui->frameLayout->addWidget(managerWorkspace);

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
  legendRow->addWidget(makeLegendPill("Thiếu", "#FEE2E2", "#B91C1C",
                                      fullTimeInfoWidget));
  legendRow->addWidget(makeLegendPill("Đủ", "#D1FAE5", "#047857",
                                      fullTimeInfoWidget));
  legendRow->addWidget(makeLegendPill("Chờ duyệt", "#FEF9C3", "#854D0E",
                                      fullTimeInfoWidget));
  legendRow->addWidget(makeLegendPill("Đã duyệt", "#A7F3D0", "#065F46",
                                      fullTimeInfoWidget));
  legendRow->addStretch();
  infoLayout->addLayout(legendRow);
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
  staffInfoStack = new QStackedWidget(this);
  staffInfoStack->setObjectName("staffInfoStack");
  staffInfoStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  staffInfoStack->setFixedHeight(92);
  staffInfoStack->addWidget(fullTimeInfoWidget);
  staffInfoStack->addWidget(partTimeInfoWidget);
  staffInfoStack->setVisible(false);
  ui->verticalLayout->insertWidget(1, staffInfoStack);

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
      new QLabel("Ca làm thiếu nhân viên", missingStaffWidget);
  lblMissingStaffHeader->setStyleSheet("font-size:16px;font-weight:700;"
                                       "color:#0F172A;padding:4px 0;background:transparent;");

  lblMissingCount = new QLabel("0 ca cần xử lý", missingStaffWidget);
  lblMissingCount->setStyleSheet(
      "background-color: #FEE2E2; color: #991B1B; border-radius: 10px; "
      "padding: 3px 10px; font-size: 12px; font-weight: bold;");

  headerRow->addWidget(lblMissingStaffHeader);
  headerRow->addStretch();
  headerRow->addWidget(lblMissingCount);
  missingLayout->addLayout(headerRow);
  QLabel *missingSubtitle = new QLabel(
      "Ưu tiên xử lý các ca có mức thiếu hụt cao nhất.", missingStaffWidget);
  missingSubtitle->setStyleSheet(
      "color:#64748B;font-size:12px;background:transparent;padding-bottom:4px;");
  missingLayout->addWidget(missingSubtitle);

  // Table
  tableMissingStaff = new QTableWidget(missingStaffWidget);
  tableMissingStaff->setColumnCount(5);
  tableMissingStaff->setHorizontalHeaderLabels(
      {"MỨC ĐỘ", "NGÀY & CA LÀM", "TÌNH TRẠNG NHÂN SỰ", "CÒN THIẾU", "HÀNH ĐỘNG"});
  tableMissingStaff->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
  tableMissingStaff->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  tableMissingStaff->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  tableMissingStaff->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
  tableMissingStaff->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
  tableMissingStaff->setColumnWidth(0, 150);
  tableMissingStaff->setColumnWidth(3, 130);
  tableMissingStaff->setColumnWidth(4, 160);
  tableMissingStaff->horizontalHeader()->setFixedHeight(46);
  tableMissingStaff->verticalHeader()->setVisible(false);
  tableMissingStaff->verticalHeader()->setDefaultSectionSize(68);
  tableMissingStaff->setSelectionMode(QAbstractItemView::SingleSelection);
  tableMissingStaff->setSelectionBehavior(QAbstractItemView::SelectRows);
  tableMissingStaff->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tableMissingStaff->setFocusPolicy(Qt::NoFocus);
  tableMissingStaff->setAlternatingRowColors(false);
  tableMissingStaff->setShowGrid(false);
  tableMissingStaff->setSortingEnabled(false);
  tableMissingStaff->setStyleSheet(
      "QTableWidget { background:#FFFFFF;border:1px solid #E2E8F0;"
      "border-radius:10px;selection-background-color:#EFF6FF; }"
      "QHeaderView::section { background:#F1F5F9;color:#475569;"
      "font-size:11px;font-weight:700;padding:10px;border:none;"
      "border-bottom:1px solid #CBD5E1; }"
      "QTableWidget::item { color:#1F2937;border:none;"
      "border-bottom:1px solid #E2E8F0;padding:8px; }"
      "QTableWidget::item:selected { background:#EFF6FF;color:#1E3A8A; }"
      "QScrollBar:vertical { background:#F8FAFC;width:10px;margin:2px; }"
      "QScrollBar::handle:vertical { background:#CBD5E1;border-radius:5px;min-height:30px; }");
  missingLayout->addWidget(tableMissingStaff);

  // Add missing staff widget to the parent layout
  QLayout *parentLayout = ui->verticalLayout;
  if (parentLayout)
  {
    parentLayout->addWidget(missingStaffWidget);
  }
}

// ─── Build the interactive time-slot grid ────────────────────────────────────

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
