#include "view/ManagerEmployeeChooser_Dialog.h"

#include <QAbstractItemView>
#include <QAbstractSpinBox>
#include <QDialogButtonBox>
#include <QSignalBlocker>
#include <QTimeEdit>

namespace {
constexpr int EmployeeIdRole = Qt::UserRole + 20;

bool isManagerRole(const QString &role)
{
    return role.compare("Manager", Qt::CaseInsensitive) == 0 ||
           role.compare("Manage", Qt::CaseInsensitive) == 0;
}

bool hasLockedBlockTime(const EligibleEmployeeInfo &employee)
{
    return employee.isFixedSalary;
}
}

ManagerEmployeeChooser_Dialog::ManagerEmployeeChooser_Dialog(
    const QList<EligibleEmployeeInfo> &employees,
    QTime blockStart, QTime blockEnd,
    const QList<ManagerEmployeeSelection> &initialSelections,
    QWidget *parent)
    : QDialog(parent), m_employees(employees),
      m_blockStart(blockStart), m_blockEnd(blockEnd)
{
    setWindowTitle(QString::fromUtf8("Chọn nhân viên"));
    setMinimumSize(820, 520);
    setModal(true);
    setStyleSheet(
        "QDialog{background:#F8FAFC;color:#1E293B;}"
        "QLineEdit,QComboBox,QTimeEdit{background:#FFFFFF;color:#1E293B;"
        "border:1px solid #CBD5E1;border-radius:6px;padding:6px;}"
        "QTableWidget{background:#FFFFFF;color:#1E293B;border:1px solid #E2E8F0;"
        "border-radius:8px;gridline-color:#E2E8F0;}"
        "QHeaderView::section{background:#F1F5F9;color:#475569;border:none;"
        "border-bottom:1px solid #CBD5E1;padding:8px;font-weight:700;}"
        "QTableWidget::item{padding:6px;}"
        "QPushButton{border-radius:6px;padding:8px 16px;font-weight:700;}");

    QMap<int, ManagerEmployeeSelection> initialById;
    for (const ManagerEmployeeSelection &selection : initialSelections)
        initialById.insert(selection.employeeId, selection);

    for (const EligibleEmployeeInfo &employee : m_employees)
    {
        if (isManagerRole(employee.role))
            continue;
        SelectionState state;
        state.startTime = m_blockStart;
        state.endTime = m_blockEnd;
        if (initialById.contains(employee.employeeId) && employee.eligible)
        {
            const ManagerEmployeeSelection initial = initialById.value(employee.employeeId);
            state.selected = true;
            if (!hasLockedBlockTime(employee))
            {
                state.startTime = initial.startTime;
                state.endTime = initial.endTime;
            }
        }
        m_states.insert(employee.employeeId, state);
    }

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(12);

    QLabel *title = new QLabel(QString::fromUtf8("CHỌN NHÂN VIÊN"), this);
    title->setStyleSheet("font-size:18px;font-weight:800;color:#0F172A;");
    QLabel *subtitle = new QLabel(
        QString::fromUtf8("Chọn nhiều nhân viên cho ca hiện tại và kiểm tra giờ làm trước khi xác nhận."),
        this);
    subtitle->setStyleSheet("color:#64748B;font-size:11px;");
    root->addWidget(title);
    root->addWidget(subtitle);

    QHBoxLayout *toolbar = new QHBoxLayout();
    toolbar->setSpacing(8);
    m_search = new QLineEdit(this);
    m_search->setObjectName("employeeSearch");
    m_search->setPlaceholderText(QString::fromUtf8("Tìm theo ID hoặc tên..."));
    m_search->setClearButtonEnabled(true);

    m_roleFilter = new QComboBox(this);
    m_roleFilter->setObjectName("employeeRoleFilter");
    m_roleFilter->addItem(QString::fromUtf8("Tất cả vai trò"), QString());
    QStringList roles;
    for (const EligibleEmployeeInfo &employee : m_employees)
        if (!isManagerRole(employee.role) &&
            !roles.contains(employee.role, Qt::CaseInsensitive))
            roles.append(employee.role);
    std::sort(roles.begin(), roles.end(), [](const QString &left, const QString &right) {
        return left.compare(right, Qt::CaseInsensitive) < 0;
    });
    for (const QString &role : roles)
        m_roleFilter->addItem(displayRole(role), role);

    m_sort = new QComboBox(this);
    m_sort->setObjectName("employeeSort");
    m_sort->addItem(QString::fromUtf8("Tên ↑"), "name_asc");
    m_sort->addItem(QString::fromUtf8("Tên ↓"), "name_desc");
    m_sort->addItem(QString::fromUtf8("ID ↑"), "id_asc");
    m_sort->addItem(QString::fromUtf8("ID ↓"), "id_desc");

    toolbar->addWidget(m_search, 1);
    toolbar->addWidget(m_roleFilter);
    toolbar->addWidget(m_sort);
    root->addLayout(toolbar);

    m_table = new QTableWidget(this);
    m_table->setObjectName("employeeChooserTable");
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels(
        {"ID", QString::fromUtf8("Tên"), QString::fromUtf8("Vai trò"),
         "Start Time", "End Time"});
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    root->addWidget(m_table, 1);

    QHBoxLayout *footer = new QHBoxLayout();
    m_selectedCount = new QLabel(this);
    m_selectedCount->setObjectName("selectedEmployeeCount");
    m_selectedCount->setStyleSheet("color:#475569;font-weight:700;");
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    m_confirmButton = buttons->addButton(QString::fromUtf8("Xác nhận"),
                                         QDialogButtonBox::AcceptRole);
    m_confirmButton->setObjectName("confirmEmployeeSelection");
    m_confirmButton->setStyleSheet(
        "QPushButton{background:#2563EB;color:#FFFFFF;border:none;}"
        "QPushButton:hover{background:#1D4ED8;}"
        "QPushButton:disabled{background:#CBD5E1;color:#64748B;}");
    footer->addWidget(m_selectedCount);
    footer->addStretch();
    footer->addWidget(buttons);
    root->addLayout(footer);

    connect(m_search, &QLineEdit::textChanged,
            this, &ManagerEmployeeChooser_Dialog::rebuildTable);
    connect(m_roleFilter, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &ManagerEmployeeChooser_Dialog::rebuildTable);
    connect(m_sort, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &ManagerEmployeeChooser_Dialog::rebuildTable);
    connect(m_table, &QTableWidget::itemChanged, this,
            [this](QTableWidgetItem *item) {
                if (!item || item->column() != 0)
                    return;
                const int employeeId = item->data(EmployeeIdRole).toInt();
                if (m_states.contains(employeeId))
                    m_states[employeeId].selected =
                        item->checkState() == Qt::Checked;
                updateConfirmState();
            });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_confirmButton, &QPushButton::clicked,
            this, &ManagerEmployeeChooser_Dialog::acceptSelection);

    rebuildTable();
}

QString ManagerEmployeeChooser_Dialog::displayRole(const QString &role) const
{
    if (role.compare("Manager", Qt::CaseInsensitive) == 0 ||
        role.compare("Manage", Qt::CaseInsensitive) == 0)
        return QString::fromUtf8("Quản lý");
    if (role.compare("Cashier", Qt::CaseInsensitive) == 0)
        return QString::fromUtf8("Thu ngân");
    if (role.compare("HallStaff", Qt::CaseInsensitive) == 0)
        return QString::fromUtf8("Nhân viên sảnh");
    if (Config::canonicalRoleName(role).compare("KitchenAssistant", Qt::CaseInsensitive) == 0)
        return QString::fromUtf8("Phụ bếp");
    return role;
}

void ManagerEmployeeChooser_Dialog::rebuildTable()
{
    QList<EligibleEmployeeInfo> visible;
    const QString search = m_search->text().trimmed();
    const QString role = m_roleFilter->currentData().toString();
    for (const EligibleEmployeeInfo &employee : m_employees)
    {
        if (isManagerRole(employee.role))
            continue;
        const QString idText = QString::number(employee.employeeId);
        const bool searchMatches = search.isEmpty() ||
            idText.contains(search, Qt::CaseInsensitive) ||
            QString("NV-%1").arg(idText).contains(search, Qt::CaseInsensitive) ||
            employee.employeeName.contains(search, Qt::CaseInsensitive);
        const bool roleMatches = role.isEmpty() ||
            employee.role.compare(role, Qt::CaseInsensitive) == 0;
        if (searchMatches && roleMatches)
            visible.append(employee);
    }

    const QString sortMode = m_sort->currentData().toString();
    std::sort(visible.begin(), visible.end(), [sortMode](
                  const EligibleEmployeeInfo &left,
                  const EligibleEmployeeInfo &right) {
        if (sortMode == "id_asc") return left.employeeId < right.employeeId;
        if (sortMode == "id_desc") return left.employeeId > right.employeeId;
        const int nameOrder = left.employeeName.compare(
            right.employeeName, Qt::CaseInsensitive);
        return sortMode == "name_desc" ? nameOrder > 0 : nameOrder < 0;
    });

    QSignalBlocker blocker(m_table);
    m_table->clearContents();
    m_table->setRowCount(visible.size());
    for (int row = 0; row < visible.size(); ++row)
    {
        const EligibleEmployeeInfo employee = visible[row];
        const SelectionState state = m_states.value(employee.employeeId);

        QTableWidgetItem *id = new QTableWidgetItem(
            QString("NV-%1").arg(employee.employeeId));
        id->setData(EmployeeIdRole, employee.employeeId);
        if (employee.eligible)
        {
            id->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
            id->setCheckState(state.selected ? Qt::Checked : Qt::Unchecked);
        }
        else
        {
            id->setFlags(Qt::NoItemFlags);
            id->setToolTip(employee.reason);
            id->setForeground(QColor("#94A3B8"));
        }
        m_table->setItem(row, 0, id);

        QTableWidgetItem *name = new QTableWidgetItem(employee.employeeName);
        QTableWidgetItem *roleItem = new QTableWidgetItem(displayRole(employee.role));
        for (QTableWidgetItem *item : {name, roleItem})
        {
            item->setFlags(employee.eligible ? Qt::ItemIsEnabled : Qt::NoItemFlags);
            if (!employee.eligible)
            {
                item->setForeground(QColor("#94A3B8"));
                item->setToolTip(employee.reason);
            }
        }
        m_table->setItem(row, 1, name);
        m_table->setItem(row, 2, roleItem);

        QTimeEdit *start = new QTimeEdit(state.startTime, m_table);
        QTimeEdit *end = new QTimeEdit(state.endTime, m_table);
        start->setObjectName(QString("startTime_%1").arg(employee.employeeId));
        end->setObjectName(QString("endTime_%1").arg(employee.employeeId));
        start->setDisplayFormat("HH:mm");
        end->setDisplayFormat("HH:mm");
        start->setButtonSymbols(QAbstractSpinBox::NoButtons);
        end->setButtonSymbols(QAbstractSpinBox::NoButtons);
        const bool editable = employee.eligible && !hasLockedBlockTime(employee);
        start->setEnabled(editable);
        end->setEnabled(editable);
        if (!employee.eligible)
        {
            start->setToolTip(employee.reason);
            end->setToolTip(employee.reason);
        }
        connect(start, &QTimeEdit::timeChanged, this,
                [this, employee](QTime time) {
                    if (!hasLockedBlockTime(employee))
                        m_states[employee.employeeId].startTime = time;
                });
        connect(end, &QTimeEdit::timeChanged, this,
                [this, employee](QTime time) {
                    if (!hasLockedBlockTime(employee))
                        m_states[employee.employeeId].endTime = time;
                });
        m_table->setCellWidget(row, 3, start);
        m_table->setCellWidget(row, 4, end);
        m_table->setRowHeight(row, 42);
    }

    updateConfirmState();
}

void ManagerEmployeeChooser_Dialog::updateConfirmState()
{
    int selected = 0;
    for (auto it = m_states.cbegin(); it != m_states.cend(); ++it)
        if (it.value().selected)
            ++selected;
    m_selectedCount->setText(
        QString::fromUtf8("Đã chọn: %1 nhân viên").arg(selected));
    m_confirmButton->setEnabled(selected > 0);
}

void ManagerEmployeeChooser_Dialog::acceptSelection()
{
    for (auto it = m_states.cbegin(); it != m_states.cend(); ++it)
    {
        if (!it.value().selected)
            continue;
        if (!it.value().startTime.isValid() || !it.value().endTime.isValid() ||
            it.value().startTime >= it.value().endTime)
        {
            QMessageBox::warning(
                this, QString::fromUtf8("Khoảng giờ không hợp lệ"),
                QString::fromUtf8("Giờ bắt đầu phải nhỏ hơn giờ kết thúc cho mọi nhân viên đã chọn."));
            return;
        }
    }
    accept();
}

QList<ManagerEmployeeSelection> ManagerEmployeeChooser_Dialog::selections() const
{
    QList<ManagerEmployeeSelection> result;
    for (const EligibleEmployeeInfo &employee : m_employees)
    {
        if (isManagerRole(employee.role))
            continue;
        const SelectionState state = m_states.value(employee.employeeId);
        if (!employee.eligible || !state.selected)
            continue;
        const bool lockedToBlock = hasLockedBlockTime(employee);
        result.append({employee.employeeId, employee.employeeName, employee.role,
                       employee.isFixedSalary,
                       lockedToBlock ? m_blockStart : state.startTime,
                       lockedToBlock ? m_blockEnd : state.endTime});
    }
    return result;
}
