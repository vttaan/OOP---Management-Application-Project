#include "Employee_Control.h"
#include "view/employeeswidget.h"
#include "view/AddEmployee_Dialog.h"
#include "view/EditEmployee_Dialog.h"



Employee_Control::Employee_Control(QObject *parent)
    : QObject(parent), m_view(nullptr)
{}

Employee_Control::~Employee_Control() {}


void Employee_Control::setView(EmployeesWidget *view)
{
    m_view = view;
    if (!m_view) return;

    connect(m_view, &EmployeesWidget::requestAddEmployee,
            this,   &Employee_Control::handleAddEmployee);

    connect(m_view, &EmployeesWidget::requestEditEmployee,
            this,   &Employee_Control::handleEditEmployee);

    connect(m_view, &EmployeesWidget::requestDeleteEmployee,
            this,   &Employee_Control::handleDeleteEmployee);

    // Combined search/filter/sort signal — replaces separate view-side logic
    connect(m_view, &EmployeesWidget::requestUpdate,
            this,   &Employee_Control::handleUpdate);
}

EmployeesWidget *Employee_Control::getView() const
{
    return m_view;
}

void Employee_Control::init()
{
    handleLoadEmployees();
}

// Slots

void Employee_Control::handleLoadEmployees()
{
    qDeleteAll(m_employees);
    m_employees.clear();

    m_employees = m_model.getAllEmployees();
    // Also sync model's internal listEmployee for its algorithm methods
    m_model.loadData();
    if (m_view) {
        // Push the full list first so metric cards show correct totals
        m_view->setFullEmployeeList(m_employees);
        m_view->loadEmployees(m_employees);
    }
}

void Employee_Control::handleAddEmployee()
{
    if (!m_view) return;

    // Open a single form dialog instead of multiple QInputDialogs
    AddEmployee_Dialog dlg(m_view);
    if (dlg.exec() != QDialog::Accepted) return;

    // Read all input from the dialog
    User *newEmp = UserFactory::createNewUser(
        dlg.getRole(), 0, dlg.getAvatarPath(),
        dlg.getCitizenId(),
        dlg.getName(),
        dlg.getDob(),
        dlg.getAddress(),
        dlg.getPhone(),
        dlg.getGender()
        );

    if (!newEmp) {
        m_view->showError("Failed to create employee object.");
        return;
    }

    // Controller calls Model to save to DB
    if (m_model.addEmployee(newEmp, dlg.getUsername(), dlg.getPassword())) {
        m_view->showSuccess(QString("Employee '%1' added successfully!").arg(dlg.getName()));
        handleLoadEmployees(); // reload table
    } else {
        m_view->showError("Failed to save employee to database.");
    }

    delete newEmp;
}

void Employee_Control::handleEditEmployee(int idEmployee)
{
    if (!m_view) return;

    // Find employee in the cache by ID
    User *emp = nullptr;
    for (User *u : m_employees) {
        if (u->getIdEmployee() == idEmployee) {
            emp = u;
            break;
        }
    }

    if (!emp) {
        m_view->showError(QString("Employee EMP-%1 not found.").arg(idEmployee));
        return;
    }

    // Open dialog pre-filled with the current data
    EditEmployee_Dialog dlg(emp, m_view);
    if (dlg.exec() != QDialog::Accepted) return;

    // Apply changes from dialog to User object
    emp->setName(dlg.getName());
    emp->setRole(dlg.getRole());
    emp->setPhoneNum(dlg.getPhone());
    emp->setDOB(dlg.getDob());
    emp->setAddress(dlg.getAddress());
    emp->setIndentityID(dlg.getCitizenId());
    emp->setAva(dlg.getAvatarPath()); // Update with new avatar path

    // Call Model to update DB
    if (m_model.updateEmployee(emp)) {
        m_view->showSuccess(QString("Employee '%1' updated successfully!").arg(emp->getName()));
        handleLoadEmployees(); // reload table
    } else {
        m_view->showError("Failed to update employee.");
    }
}

void Employee_Control::handleDeleteEmployee(int idEmployee)
{
    if (!m_view) return;

    // Controller asks for confirmation
    auto reply = QMessageBox::question(
        m_view, "Confirm Delete",
        QString("Delete employee EMP-%1?").arg(idEmployee),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    // Controller calls Model
    if (m_model.deleteEmployee(idEmployee)) {
        qDebug() << "Deleted employee id=" << idEmployee;
        handleLoadEmployees();
    } else {
        m_view->showError("Failed to delete employee.");
    }
}

// ============================================================
// Combined filter → search → sort pipeline (Rabin-Karp + stable_sort)
// ============================================================

void Employee_Control::handleUpdate(const QString &searchText,
                                    const QStringList &roles,
                                    const QStringList &genders,
                                    const QString &sortField,
                                    int sortDir)
{
    if (!m_view) return;

    // Step 1: Role + Gender filter (AND between categories, OR within)
    QList<User *> result = m_employees;
    bool doRole   = !roles.isEmpty();
    bool doGender = !genders.isEmpty();
    if (doRole || doGender) {
        QList<User *> filtered;
        for (User *u : result) {
            bool roleOk   = !doRole   || roles.contains(u->getRole());
            bool genderOk = !doGender || genders.contains(u->getGender());
            if (roleOk && genderOk) filtered.append(u);
        }
        result = filtered;
    }

    // Step 2: Rabin-Karp text search on name, role, and ID
    if (!searchText.isEmpty()) {
        QList<User *> searched;
        for (User *u : result) {
            if (rabinKarp(searchText, u->getName()) ||
                rabinKarp(searchText, u->getRole()) ||
                rabinKarp(searchText, QString::number(u->getIdEmployee())))
                searched.append(u);
        }
        result = searched;
    }

    // Step 3: stable_sort on filtered+searched result
    if (!sortField.isEmpty() && sortDir != 0) {
        QStringList fields = {sortField};
        std::stable_sort(result.begin(), result.end(), [&](User *a, User *b) {
            return sortDir == 1 ? cmpAscending(a, b, fields)
                                : cmpDescending(a, b, fields);
        });
    }

    m_view->setFullEmployeeList(m_employees); // totals always from full cache
    m_view->loadEmployees(result);
}

// ============================================================
// Controller algorithm helpers (declared in header)
// ============================================================

bool Employee_Control::rabinKarp(const QString &pattern, const QString &text)
{
    QString p = pattern.toLower();
    QString t = text.toLower();
    int n = t.length(), m = p.length();
    if (m == 0) return true;
    if (m > n)  return false;

    const long long BASE = 31, MOD = 1000000007LL;
    long long pHash = 0, tHash = 0, power = 1;

    for (int i = 0; i < m; ++i) {
        pHash = (pHash * BASE + p[i].unicode()) % MOD;
        tHash = (tHash * BASE + t[i].unicode()) % MOD;
        if (i > 0) power = (power * BASE) % MOD;
    }
    if (pHash == tHash && t.mid(0, m) == p) return true;

    for (int i = 1; i <= n - m; ++i) {
        tHash = (tHash - t[i - 1].unicode() * power % MOD + MOD) % MOD;
        tHash = (tHash * BASE + t[i + m - 1].unicode()) % MOD;
        if (pHash == tHash && t.mid(i, m) == p) return true;
    }
    return false;
}

bool Employee_Control::cmpAscending(User *a, User *b, const QStringList &contentSort)
{
    if (contentSort.isEmpty()) return a->getIdEmployee() < b->getIdEmployee();
    if (contentSort[0] == "name") return a->getName() < b->getName();
    return a->getIdEmployee() < b->getIdEmployee();
}

bool Employee_Control::cmpDescending(User *a, User *b, const QStringList &contentSort)
{
    if (contentSort.isEmpty()) return a->getIdEmployee() > b->getIdEmployee();
    if (contentSort[0] == "name") return a->getName() > b->getName();
    return a->getIdEmployee() > b->getIdEmployee();
}

// Public wrapper methods (declared in header for external callers)
QList<User *> Employee_Control::searchInEmployee(const QString &contentSearch)
{
    QList<User *> result;
    for (User *u : m_employees)
        if (rabinKarp(contentSearch, u->getName()) ||
            rabinKarp(contentSearch, u->getRole()) ||
            rabinKarp(contentSearch, QString::number(u->getIdEmployee())))
            result.append(u);
    return result;
}

QList<User *> Employee_Control::filterInEmployee(const QStringList &contentFilter)
{
    QList<User *> result;
    for (User *u : m_employees)
        for (const QString &f : contentFilter)
            if (u->getRole() == f || u->getGender() == f) {
                result.append(u);
                break;
            }
    return result;
}

QList<User *> Employee_Control::sortInEmployee(const QString &typeOrder, const QStringList &contentSort)
{
    QList<User *> sorted = m_employees;
    std::stable_sort(sorted.begin(), sorted.end(), [&](User *a, User *b) {
        return typeOrder == "asc" ? cmpAscending(a, b, contentSort)
                                 : cmpDescending(a, b, contentSort);
    });
    return sorted;
}
