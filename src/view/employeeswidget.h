#ifndef EMPLOYEESWIDGET_H
#define EMPLOYEESWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFrame>
#include <QSizePolicy>
#include <QFont>
#include <QEvent>
#include "model/Employee_Model.h"

class EmployeesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EmployeesWidget(QWidget *parent = nullptr);
    ~EmployeesWidget();

signals:
    void profileClicked();

    // Signals sent to the Controller — CRUD
    void requestAddEmployee();
    void requestEditEmployee(int idEmployee);
    void requestDeleteEmployee(int idEmployee);

    // Combined signal: emitted whenever search/filter/sort criteria change.
    // The Controller receives it, applies filter→search→sort pipeline on its
    // m_employees, then calls loadEmployees() with the result.
    void requestUpdate(const QString &searchText,
                       const QList<QString> &contentFilter,
                       const QList<QString> &contentSort,
                       int sortDir); // 0=none, 1=asc, -1=desc

public slots:
    // Called by the Controller to push data to the view
    void loadEmployees(const QList<User *> &employees);
    void showError(const QString &msg);
    void showSuccess(const QString &msg);

private slots:
    void handleAddEmployee();
    // Emits combined requestUpdate with all active criteria
    void emitUpdateRequest();
    // Filter dropdown toggle
    void toggleFilterDropdown();
    // Sort dropdown
    void toggleSortDropdown();
    void onSortFieldSelected(const QString &field);
    void cycleSortDirection();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupUi();
    void setupTableHeader();
    void setupConnections();
    void buildFilterDropdown();
    void buildSortDropdown();

    // Renders table rows from a given employee list
    void renderTable(const QList<User *> &employees);

    // --- Widget Factories ---
    QLabel      *createAvatar(const QString &initials, const QString &bgColor);
    QFrame      *createMetricCard(const QString &iconText,
                                  const QString &iconBg,
                                  const QString &iconColor,
                                  const QString &title,
                                  const QString &value,
                                  const QString &subtitle,
                                  const QString &badge      = QString(),
                                  const QString &badgeColor = QString());
    QLabel      *createStatusBadge(const QString &status);
    QLabel      *createRoleBadge(const QString &role);
    QPushButton *createActionButton(const QString &iconPath, const QString &tooltip);

    // --- Profile Block (top-right) ---
    QFrame *profileBlock;

    // --- Metric Cards Row ---
    QHBoxLayout *metricsLayout;

    // --- Roster Card ---
    QFrame       *rosterCard;
    QLineEdit    *searchRoster;
    QPushButton  *filterBtn;
    QPushButton  *sortBtn;
    QPushButton  *addEmployeeBtn;
    QTableWidget *employeesTable;
    QLabel       *footerLabel;

    // --- Filter Dropdown (floating overlay, child of EmployeesWidget) ---
    QFrame    *filterDropdown;
    QCheckBox *chkStaff;
    QCheckBox *chkManager;
    QCheckBox *chkAdmin;
    QCheckBox *chkMale;
    QCheckBox *chkFemale;
    bool       m_filterOpen = false;

    // --- Sort Dropdown (floating overlay, child of EmployeesWidget) ---
    QFrame    *sortDropdown;
    QFrame    *sortTagBar;   // tag chip row shown below roster header
    QLabel    *sortTagLabel; // clickable tag label
    bool       m_sortOpen  = false;
    QString    m_sortField;  // "" | "id" | "name"
    int        m_sortDir   = 0; // 0=none, 1=asc, -1=desc

    // --- Main Layout ---
    QVBoxLayout *mainLayout;
};

#endif // EMPLOYEESWIDGET_H
