#include "global.h"
#include "Employee_Control.h"
#include "view/employeeswidget.h"
#include "view/AddEmployee_Dialog.h"
#include "view/EditEmployee_Dialog.h"

Employee_Control::Employee_Control(QObject *parent)
    : QObject(parent), m_view(nullptr), m_model(new Employee_Model())
{}

Employee_Control::~Employee_Control() {delete m_model;}

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

    connect(m_view, &EmployeesWidget::requestUpdate,
            this, &Employee_Control::handleUpdate);

    connect(m_view, &EmployeesWidget::profileClicked,
            this, &Employee_Control::profilePageClicked);

        connect(m_view, &EmployeesWidget::backToDashboard,
            this, &Employee_Control::backToDashBoard);
}

void Employee_Control::setModel(Employee_Model* emp) {
    this->m_model = emp;
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
    m_model->loadData();
    if (m_view) {
        //m_view->c(m_model->getListEmployee());
          m_view->loadEmployees(m_model->getListEmployee());
    }
}

void Employee_Control::handleAddEmployee()
{
    if (!m_view) return;

    // Open a single form dialog instead of multiple QInputDialogs
    AddEmployee_Dialog dlg(m_view);
    if (dlg.exec() != QDialog::Accepted) return;

    if(m_model->addEmployee(dlg.getRole(), dlg.getAvatarPath(), dlg.getCitizenId(), dlg.getName(),
     dlg.getDob(), dlg.getAddress(), dlg.getPhone(), dlg.getGender(), dlg.getUsername(), dlg.getPassword())) {
        m_view->showSuccess(QString("THÊM NHÂN VIÊN %1 THÀNH CÔNG").arg(dlg.getName()));
        handleLoadEmployees();
    }
    else m_view->showError(QString("THÊM NHÂN VIÊN %1 THẤT BẠI").arg(dlg.getName()));
}

void Employee_Control::handleEditEmployee(int idEmployee)
{
    if (!m_view) return;

    // Find employee in the cache by ID
    User *emp = nullptr;
    for (User *u : m_model->getListEmployee()) {
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
    emp->setGender(dlg.getGender());
    // Call Model to update DB
    if (m_model->updateEmployee(emp)) {
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
    if (m_model->deleteEmployee(idEmployee)) {
        qDebug() << "Deleted employee id=" << idEmployee;
        handleLoadEmployees();
    } else {
        m_view->showError("Failed to delete employee.");
    }
}

// ============================================================
// Combined filter → search → sort pipeline
// ============================================================

void Employee_Control::handleUpdate(const QString &searchText,
                                    const QList<QString> &contentFilter,
                                    const QList<QString> &contentSort,
                                    short sortDir)
{
    if (!m_view || !m_model) return;
    QList<User*> result = m_model->SearchSortFilter(searchText, sortDir, contentSort, contentFilter);

    m_view->loadEmployees(result);
}

