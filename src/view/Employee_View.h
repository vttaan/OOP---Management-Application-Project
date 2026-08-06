#include "global.h"
#ifndef EMPLOYEE_VIEW_H
#define EMPLOYEE_VIEW_H


namespace Ui { class Employee_View; }

class Employee_View : public QWidget
{
    Q_OBJECT
public:
    explicit Employee_View(QWidget *parent = nullptr);
    ~Employee_View();

signals:
    void backToDashboard();
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
    void loadEmployees(const QList<User *> &employees, const QMap<int, double> &hoursMap);

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

private:
    // ---- ui pointer (owns all widgets declared in the .ui file) ----
    Ui::Employee_View *ui;

    void setupTableHeader();
    void setupConnections();
    void buildFilterDropdown();
    void buildSortDropdown();



    // --- Widget Factories ---
    QLabel      *createAvatar(const QString &avatarPath);
    QLabel      *createRoleBadge(const QString &role);
    QLabel      *createPayTypeBadge(const QString &payType);
    QPushButton *createActionButton(const QString &iconPath, const QString &tooltip);

    // Full unfiltered employee list (for metric card totals)
    QList<User *> m_allEmployees;

    // --- Filter Dropdown (floating overlay, child of Employee_View) ---
    QFrame    *filterDropdown;
    QCheckBox *chkCashier;
    QCheckBox *chkHallStaff;
    QCheckBox *chkKitchenAssistant;
    QCheckBox *chkManager;
    QCheckBox *chkAdmin;
    QCheckBox *chkMale;
    QCheckBox *chkFemale;
    bool       m_filterOpen = false;

    // --- Sort Dropdown (floating overlay, child of Employee_View) ---
    QFrame    *sortDropdown;
    bool       m_sortOpen  = false;
    QString    m_sortField;  // "" | "id" | "name"
    int        m_sortDir   = 0; // 0=none, 1=asc, -1=desc

    // --- Search debounce timer
    QTimer    *m_searchTimer = nullptr;

    // --- Avatar pixmap cache
    QHash<QString, QPixmap> m_avatarCache;
};

#endif // EMPLOYEE_VIEW_H
