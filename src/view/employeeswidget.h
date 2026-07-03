#ifndef EMPLOYEESWIDGET_H
#define EMPLOYEESWIDGET_H

#include <QWidget>
#include <QSqlTableModel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFrame>
#include <QScrollArea>
#include <QTimer>
#include <QDateTime>
#include <QSizePolicy>
#include <QFont>
#include <QFile>
#include <QTextStream>

class EmployeesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EmployeesWidget(QWidget *parent = nullptr);
    ~EmployeesWidget();

private slots:
    void filterEmployees(const QString &searchText);
    void handleAddEmployee();
    void updateClock();

private:
    // --- Setup Methods ---
    void setupUi();
    void setupTableHeader();
    void populateTable();
    void setupConnections();
    void logEvent(const QString &eventType, const QString &details);

    // --- Helper Widget Factories ---
    QLabel*      createAvatar(const QString &initials, const QString &bgColor);
    QFrame*      createMetricCard(const QString &iconEmoji,
                                  const QString &iconBg,
                                  const QString &iconColor,
                                  const QString &title,
                                  const QString &value,
                                  const QString &subtitle,
                                  const QString &badge      = QString(),
                                  const QString &badgeColor = QString());
    QLabel*      createStatusBadge(const QString &status);
    QLabel*      createRoleBadge(const QString &role);
    QPushButton* createActionButton(const QString &emoji, const QString &tooltip);

    // --- Right-side Info Panels ---
    QFrame*      createInfoPanel(const QString &title,
                                 const QString &iconEmoji,
                                 const QString &objectName);

    // --- Top Bar ---
    QLabel    *lblBreadcrumb;
    QLineEdit *searchTopBar;
    QLabel    *lblClock;
    QTimer    *clockTimer;

    // --- Metric Cards Row ---
    QHBoxLayout *metricsLayout;

    // --- Main content row (table + right panels) ---
    QHBoxLayout *contentRowLayout;

    // --- Roster Card (left) ---
    QFrame     *rosterCard;
    QLineEdit  *searchRoster;
    QPushButton *addEmployeeBtn;
    QTableWidget *employeesTable;
    QLabel      *footerLabel;

    // --- Right Info Panels ---
    QFrame      *nextShiftPanel;
    QFrame      *absentTodayPanel;

    // --- Main Layout ---
    QVBoxLayout *mainLayout;
};

#endif // EMPLOYEESWIDGET_H
