#ifndef EMPLOYEESWIDGET_H
#define EMPLOYEESWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFrame>
#include <QTimer>
#include <QDateTime>
#include <QSizePolicy>
#include <QFont>
#include <QEvent>

class EmployeesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EmployeesWidget(QWidget *parent = nullptr);
    ~EmployeesWidget();

signals:
    void profileClicked();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void filterEmployees(const QString &searchText);
    void applyRoleFilter(int index);
    void handleAddEmployee();

private:
    // --- Setup ---
    void setupUi();
    void setupTableHeader();
    void populateTable();
    void setupConnections();

    // --- Widget Factories ---
    QLabel*      createAvatar(const QString &initials, const QString &bgColor);
    QFrame*      createMetricCard(const QString &iconText,
                                  const QString &iconBg,
                                  const QString &iconColor,
                                  const QString &title,
                                  const QString &value,
                                  const QString &subtitle,
                                  const QString &badge      = QString(),
                                  const QString &badgeColor = QString());
    QLabel*      createStatusBadge(const QString &status);
    QLabel*      createRoleBadge(const QString &role);
    QPushButton* createActionButton(const QString &text, const QString &tooltip);

    // --- Profile Block (top-right) ---
    QFrame      *profileBlock;

    // --- Metric Cards Row ---
    QHBoxLayout *metricsLayout;

    // --- Roster Card ---
    QFrame      *rosterCard;
    QLineEdit   *searchRoster;
    QComboBox   *filterCombo;
    QPushButton *sortBtn;
    QPushButton *addEmployeeBtn;
    QTableWidget *employeesTable;
    QLabel      *footerLabel;

    // --- Main Layout ---
    QVBoxLayout *mainLayout;
};

#endif // EMPLOYEESWIDGET_H
