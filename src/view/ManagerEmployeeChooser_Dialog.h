#pragma once

#include "global.h"
#include "utils/ScheduleDTOs.h"

class QDialogButtonBox;
class QTimeEdit;

class ManagerEmployeeChooser_Dialog final : public QDialog
{
    Q_OBJECT

public:
    explicit ManagerEmployeeChooser_Dialog(
        const QList<EligibleEmployeeInfo> &employees,
        QTime blockStart, QTime blockEnd,
        const QList<ManagerEmployeeSelection> &initialSelections = {},
        QWidget *parent = nullptr);

    QList<ManagerEmployeeSelection> selections() const;

private slots:
    void rebuildTable();
    void acceptSelection();

private:
    struct SelectionState
    {
        bool selected = false;
        QTime startTime;
        QTime endTime;
    };

    QString displayRole(const QString &role) const;
    void updateConfirmState();

    QList<EligibleEmployeeInfo> m_employees;
    QMap<int, SelectionState> m_states;
    QTime m_blockStart;
    QTime m_blockEnd;

    QLineEdit *m_search = nullptr;
    QComboBox *m_roleFilter = nullptr;
    QComboBox *m_sort = nullptr;
    QTableWidget *m_table = nullptr;
    QLabel *m_selectedCount = nullptr;
    QPushButton *m_confirmButton = nullptr;
};
