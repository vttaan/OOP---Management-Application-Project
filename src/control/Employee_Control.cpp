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
    if (m_view)
        m_view->loadEmployees(m_employees);
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
        dlg.getPhone()
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
        // Controller reloads the view
        handleLoadEmployees();
    } else {
        m_view->showError("Failed to delete employee.");
    }
}
