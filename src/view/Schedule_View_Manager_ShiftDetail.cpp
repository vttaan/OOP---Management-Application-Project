#include "Schedule_View.h"
#include "view/ManagerEmployeeChooser_Dialog.h"
#include "global.h"
#include <QScrollArea>

namespace {
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

  const bool sameSelectionBlock =
      m_managerSelectionDate == shiftDate &&
      m_managerSelectionStart == blockStart &&
      m_managerSelectionEnd == blockEnd;
  if (!sameSelectionBlock)
      m_managerEmployeeSelections.clear();
  m_managerSelectionDate = shiftDate;
  m_managerSelectionStart = blockStart;
  m_managerSelectionEnd = blockEnd;

  if (shiftDetailDrawer && shiftDetailDrawerLayout)
  {
    activeManagerAddButton = nullptr;
    managerAddRejectedDuringRequest = false;
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
    auto addEmployeeCard = [this, roleDisplayName](
                               QWidget *host, QVBoxLayout *targetLayout,
                               const PendingShiftInfo &info, bool pending,
                               QDialog *popup = nullptr) {
      QFrame *card = new QFrame(host);
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
                  approve->setVisible(false);
                  decline->setText("Đã từ chối");
                  emit requestDeclineShift(info);
                });
        connect(approve, &QPushButton::clicked, this,
                [this, info, decline, approve]() {
                  decline->setVisible(false);
                  approve->setEnabled(false);
                  approve->setText("Đã duyệt");
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
        connect(remove, &QPushButton::clicked, this, [this, info, remove, host, popup]() {
          bool ok = false;
          QString reason = QInputDialog::getText(
              host, "Gỡ nhân viên khỏi ca", "Lý do:",
              QLineEdit::Normal, "Điều chỉnh phân ca", &ok);
          if (ok && !reason.trimmed().isEmpty())
          {
            remove->setEnabled(false);
            remove->setText("Đã gỡ");
            emit requestRemoveAssignedShift(info.shiftId, info.employeeId, reason.trimmed());
            if (popup)
              popup->accept();
          }
        });
        actions->addWidget(remove);
      }
      cardLayout->addLayout(actions, 0);
      targetLayout->addWidget(card);
    };

    auto showFullSection = [this, addEmployeeCard](
                               const QString &sectionName,
                               const QList<PendingShiftInfo> &sectionRequests,
                               bool pending) {
      QDialog *popup = new QDialog(this);
      popup->setWindowTitle(QString("Chi tiết - %1").arg(sectionName));
      popup->setMinimumSize(620, 420);
      popup->setWindowFlags(Qt::Tool | Qt::WindowTitleHint |
                            Qt::WindowCloseButtonHint);
      popup->setAttribute(Qt::WA_DeleteOnClose, true);
      popup->setStyleSheet("QDialog { background:#F8FAFC; }");

      QVBoxLayout *popupLayout = new QVBoxLayout(popup);
      popupLayout->setContentsMargins(18, 18, 18, 18);
      popupLayout->setSpacing(10);

      QLabel *popupTitle = new QLabel(sectionName, popup);
      popupTitle->setStyleSheet(
          "color:#0F172A;font-size:16px;font-weight:800;");
      QLabel *popupCount = new QLabel(
          QString("%1 nhân viên").arg(sectionRequests.size()), popup);
      popupCount->setStyleSheet("color:#64748B;font-size:11px;");
      popupLayout->addWidget(popupTitle);
      popupLayout->addWidget(popupCount);

      QScrollArea *popupScroll = new QScrollArea(popup);
      popupScroll->setWidgetResizable(true);
      popupScroll->setFrameShape(QFrame::NoFrame);
      popupScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      popupScroll->setStyleSheet(
          "QScrollArea { background:#FFFFFF;border:1px solid #E2E8F0;"
          "border-radius:10px; }"
          "QScrollBar:vertical { background:#F8FAFC;width:7px;"
          "border-radius:3px; }"
          "QScrollBar::handle:vertical { background:#CBD5E1;"
          "border-radius:3px;min-height:28px; }"
          "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical {"
          "height:0; }");
      QWidget *popupBody = new QWidget(popupScroll);
      popupBody->setStyleSheet("background:#FFFFFF;");
      QVBoxLayout *popupBodyLayout = new QVBoxLayout(popupBody);
      popupBodyLayout->setContentsMargins(10, 10, 10, 10);
      popupBodyLayout->setSpacing(8);
      for (const PendingShiftInfo &request : sectionRequests)
        addEmployeeCard(popupBody, popupBodyLayout, request, pending, popup);
      popupBodyLayout->addStretch();
      popupScroll->setWidget(popupBody);
      popupLayout->addWidget(popupScroll, 1);

      QPushButton *close = new QPushButton("Đóng", popup);
      close->setFixedWidth(90);
      close->setStyleSheet(
          "QPushButton { background:#64748B;color:white;border:none;"
          "border-radius:6px;padding:7px 14px;font-weight:700; }"
          "QPushButton:hover { background:#475569; }");
      QHBoxLayout *closeRow = new QHBoxLayout();
      closeRow->addStretch();
      closeRow->addWidget(close);
      popupLayout->addLayout(closeRow);
      connect(close, &QPushButton::clicked, popup, &QDialog::accept);

      popup->show();
    };

    auto addMoreLink = [scrollBody, bodyLayout, showFullSection](
                           const QString &sectionName,
                           const QList<PendingShiftInfo> &sectionRequests,
                           bool pending) {
      if (sectionRequests.size() <= 2)
        return;
      QPushButton *more = new QPushButton("Xem thêm", scrollBody);
      more->setCursor(Qt::PointingHandCursor);
      more->setStyleSheet(
          "QPushButton { background:transparent;color:#2563EB;border:none;"
          "padding:2px 0;font-size:10px;text-align:left; }"
          "QPushButton:hover { color:#1D4ED8;text-decoration:underline; }");
      connect(more, &QPushButton::clicked, scrollBody,
              [showFullSection, sectionName, sectionRequests, pending]() {
                showFullSection(sectionName, sectionRequests, pending);
              });
      bodyLayout->addWidget(more, 0, Qt::AlignLeft);
    };

    int visibleApproved = 0;
    int visiblePending = 0;
    for (const PendingShiftInfo &request : requests)
    {
      if (!matchesRole(request.role)) continue;
      if (request.status == 1) ++visibleApproved;
      else if (request.status == 0) ++visiblePending;
    }

    QList<PendingShiftInfo> approvedRequests;
    QList<PendingShiftInfo> pendingRequests;
    for (const PendingShiftInfo &request : requests)
    {
      if (!matchesRole(request.role)) continue;
      if (request.status == 1) approvedRequests.append(request);
      else if (request.status == 0) pendingRequests.append(request);
    }

    bodyLayout->addWidget(sectionTitle("Nhân viên đã xếp", visibleApproved));
    for (int i = 0; i < qMin(2, approvedRequests.size()); ++i)
      addEmployeeCard(scrollBody, bodyLayout, approvedRequests[i], false);
    addMoreLink("Nhân viên đã xếp", approvedRequests, false);
    if (visibleApproved == 0)
      addEmptyState("Chưa có nhân viên phù hợp với bộ lọc.");

    bodyLayout->addWidget(sectionTitle("Yêu cầu chờ duyệt", visiblePending));
    for (int i = 0; i < qMin(2, pendingRequests.size()); ++i)
      addEmployeeCard(scrollBody, bodyLayout, pendingRequests[i], true);
    addMoreLink("Yêu cầu chờ duyệt", pendingRequests, true);
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
    QLabel *addStaffHint = new QLabel(
        "Tìm kiếm, lọc và chọn nhiều nhân viên; nhân viên xung đột sẽ bị khóa.",
        addStaffPanel);
    addStaffHint->setWordWrap(true);
    addStaffHint->setStyleSheet("color:#64748B;font-size:10px;");
    addStaffLayout->addWidget(addStaffTitle);
    addStaffLayout->addWidget(addStaffHint);

    QPushButton *chooseEmployees = new QPushButton(
        QString::fromUtf8("Chọn nhân viên"), addStaffPanel);
    chooseEmployees->setStyleSheet(
        "QPushButton { background:#FFFFFF;color:#1D4ED8;border:1px solid #93C5FD;"
        "border-radius:6px;padding:8px 12px;font-weight:700;text-align:left; }"
        "QPushButton:hover { background:#EFF6FF; }");
    addStaffLayout->addWidget(chooseEmployees);

    QTableWidget *selectedQueue = new QTableWidget(addStaffPanel);
    selectedQueue->setObjectName("managerSelectedEmployeeQueue");
    selectedQueue->setColumnCount(4);
    selectedQueue->setHorizontalHeaderLabels(
        {"ID", QString::fromUtf8("Tên"), QString::fromUtf8("Vai trò"),
         QString::fromUtf8("Giờ làm")});
    selectedQueue->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    selectedQueue->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    selectedQueue->verticalHeader()->setVisible(false);
    selectedQueue->setEditTriggers(QAbstractItemView::NoEditTriggers);
    selectedQueue->setSelectionMode(QAbstractItemView::NoSelection);
    selectedQueue->setMaximumHeight(150);
    selectedQueue->setStyleSheet(
        "QTableWidget { background:#FFFFFF;color:#334155;border:1px solid #E2E8F0;"
        "border-radius:6px;gridline-color:#E2E8F0;font-size:10px; }"
        "QHeaderView::section { background:#F1F5F9;color:#64748B;border:none;"
        "padding:5px;font-weight:700; }");
    addStaffLayout->addWidget(selectedQueue);

    QPushButton *addButton = new QPushButton(
        QString::fromUtf8("+ Thêm nhân viên vào ca"), addStaffPanel);
    activeManagerAddButton = addButton;
    addButton->setStyleSheet(
        "QPushButton { background:#2563EB;color:white;border:none;border-radius:6px;"
        "padding:9px 12px;font-weight:700; }"
        "QPushButton:hover { background:#1D4ED8; }"
        "QPushButton:disabled { background:#E2E8F0;color:#94A3B8; }");
    addStaffLayout->addWidget(addButton);

    auto populateSelectedQueue =
        [this, selectedQueue, addButton, roleDisplayName]() {
          selectedQueue->setRowCount(m_managerEmployeeSelections.size());
          for (int row = 0; row < m_managerEmployeeSelections.size(); ++row)
          {
            const ManagerEmployeeSelection selection =
                m_managerEmployeeSelections[row];
            selectedQueue->setItem(
                row, 0, new QTableWidgetItem(
                    QString("NV-%1").arg(selection.employeeId)));
            selectedQueue->setItem(
                row, 1, new QTableWidgetItem(selection.employeeName));
            selectedQueue->setItem(
                row, 2, new QTableWidgetItem(roleDisplayName(selection.role)));
            selectedQueue->setItem(
                row, 3, new QTableWidgetItem(
                    QString("%1-%2").arg(selection.startTime.toString("HH:mm"),
                                          selection.endTime.toString("HH:mm"))));
            selectedQueue->setRowHeight(row, 30);
          }
          selectedQueue->setVisible(!m_managerEmployeeSelections.isEmpty());
          addButton->setEnabled(!m_managerEmployeeSelections.isEmpty());
        };

    connect(chooseEmployees, &QPushButton::clicked, this,
            [this, eligibleEmployees, shiftDate, blockStart, blockEnd,
             populateSelectedQueue, addButton]() {
      ManagerEmployeeChooser_Dialog chooser(
          eligibleEmployees, blockStart, blockEnd,
          m_managerEmployeeSelections, shiftDetailDrawer);
      if (chooser.exec() != QDialog::Accepted)
        return;
      m_managerEmployeeSelections = chooser.selections();
      addButton->setText(QString::fromUtf8("+ Thêm nhân viên vào ca"));
      populateSelectedQueue();
    });

    connect(addButton, &QPushButton::clicked, this,
            [this, shiftDate, blockStart, blockEnd, addButton]() {
      if (m_managerEmployeeSelections.isEmpty())
        return;
      managerAddRejectedDuringRequest = false;
      emit requestAddEmployees(shiftDate, blockStart, blockEnd,
                               m_managerEmployeeSelections);
      if (managerAddRejectedDuringRequest)
        return;
      addButton->setText(QString::fromUtf8("✓ Đã thêm vào bản nháp"));
      addButton->setEnabled(false);
    });
    populateSelectedQueue();
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

  QTableWidget *selectedQueue = new QTableWidget(dlg);
  selectedQueue->setObjectName("managerSelectedEmployeeQueueDialog");
  selectedQueue->setColumnCount(4);
  selectedQueue->setHorizontalHeaderLabels(
      {"ID", QString::fromUtf8("Tên"), QString::fromUtf8("Vai trò"),
       QString::fromUtf8("Giờ làm")});
  selectedQueue->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  selectedQueue->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  selectedQueue->verticalHeader()->setVisible(false);
  selectedQueue->setEditTriggers(QAbstractItemView::NoEditTriggers);
  selectedQueue->setSelectionMode(QAbstractItemView::NoSelection);
  selectedQueue->setMaximumHeight(150);
  mainLayout->addWidget(selectedQueue);

  QHBoxLayout *selectionRow = new QHBoxLayout();
  QPushButton *chooseEmployees = new QPushButton(
      QString::fromUtf8("Chọn nhân viên"), dlg);
  QPushButton *confirmAdd = new QPushButton(
      QString::fromUtf8("+ Thêm nhân viên vào ca"), dlg);
  activeManagerAddButton = confirmAdd;
  chooseEmployees->setStyleSheet(
      "QPushButton { background:#FFFFFF;color:#1D4ED8;border:1px solid #93C5FD;"
      "border-radius:6px;padding:7px 14px;font-weight:bold; }");
  confirmAdd->setStyleSheet(
      "QPushButton { background:#2563EB;color:white;border:none;border-radius:6px;"
      "padding:7px 14px;font-weight:bold; }"
      "QPushButton:disabled { background:#CBD5E1;color:#64748B; }");
  selectionRow->addWidget(chooseEmployees);
  selectionRow->addWidget(confirmAdd);
  selectionRow->addStretch();
  mainLayout->addLayout(selectionRow);

  auto populateSelectedQueue = [this, selectedQueue, confirmAdd]() {
    selectedQueue->setRowCount(m_managerEmployeeSelections.size());
    for (int row = 0; row < m_managerEmployeeSelections.size(); ++row)
    {
      const ManagerEmployeeSelection selection = m_managerEmployeeSelections[row];
      selectedQueue->setItem(
          row, 0, new QTableWidgetItem(QString("NV-%1").arg(selection.employeeId)));
      selectedQueue->setItem(row, 1,
                             new QTableWidgetItem(selection.employeeName));
      selectedQueue->setItem(row, 2, new QTableWidgetItem(selection.role));
      selectedQueue->setItem(
          row, 3, new QTableWidgetItem(
              QString("%1-%2").arg(selection.startTime.toString("HH:mm"),
                                    selection.endTime.toString("HH:mm"))));
    }
    selectedQueue->setVisible(!m_managerEmployeeSelections.isEmpty());
    confirmAdd->setEnabled(!m_managerEmployeeSelections.isEmpty());
  };

  connect(chooseEmployees, &QPushButton::clicked, this,
          [this, dlg, eligibleEmployees, blockStart, blockEnd,
           populateSelectedQueue, confirmAdd]() {
    ManagerEmployeeChooser_Dialog chooser(
        eligibleEmployees, blockStart, blockEnd,
        m_managerEmployeeSelections, dlg);
    if (chooser.exec() != QDialog::Accepted)
      return;
    m_managerEmployeeSelections = chooser.selections();
    confirmAdd->setText(QString::fromUtf8("+ Thêm nhân viên vào ca"));
    populateSelectedQueue();
  });

  connect(confirmAdd, &QPushButton::clicked, this,
          [this, shiftDate, blockStart, blockEnd, confirmAdd]() {
    if (m_managerEmployeeSelections.isEmpty())
      return;
    managerAddRejectedDuringRequest = false;
    emit requestAddEmployees(shiftDate, blockStart, blockEnd,
                             m_managerEmployeeSelections);
    if (managerAddRejectedDuringRequest)
      return;
    confirmAdd->setText(QString::fromUtf8("✓ Đã thêm vào bản nháp"));
    confirmAdd->setEnabled(false);
  });
  populateSelectedQueue();

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


