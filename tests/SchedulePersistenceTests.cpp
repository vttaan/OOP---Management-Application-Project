#include <QtTest>
#include <QAbstractSpinBox>
#include <QTimeEdit>
#include <memory>

#include "model/Login_Model.h"
#include "model/Change_password.h"
#include "model/Salary_Model.h"
#include "model/Schedule_Model.h"
#include "model/Notification_Model.h"
#include "model/LeaveRequest_Model.h"
#include "view/ManagerEmployeeChooser_Dialog.h"
#include "utils/Config.h"
#include "utils/Security.h"
#include "utils/Database.h"

class SchedulePersistenceTests final : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir temporaryDirectory;
    QDate weekStart{2026, 8, 3};

    QSqlDatabase database() const
    {
        return Database::getInstance()->getDbConnect();
    }

    bool execute(const QString &sql)
    {
        QSqlQuery query(database());
        if (query.exec(sql))
            return true;
        qWarning() << query.lastError().text() << sql;
        return false;
    }

    int scalar(const QString &sql)
    {
        QSqlQuery query(database());
        if (!query.exec(sql) || !query.next())
            return -1;
        return query.value(0).toInt();
    }

private slots:
    void initTestCase()
    {
        Config::setOpenHour(7);
        Config::setCloseHour(22);
        Config::setDayOpenRegisShift(Qt::Tuesday);
        QVERIFY(temporaryDirectory.isValid());
        QDir root(temporaryDirectory.path());
        QVERIFY(root.mkdir("database"));
        QVERIFY(QDir::setCurrent(temporaryDirectory.path()));

        QVERIFY(execute(
            "CREATE TABLE PROFILES ("
            "idEmployee INTEGER PRIMARY KEY, role TEXT NOT NULL, "
            "IdCitizenIdentity TEXT, name TEXT, phoneNum TEXT, dob TEXT, "
            "address TEXT, avatarPath TEXT, Gender TEXT, Salary INTEGER, "
            "isFixed INTEGER DEFAULT 0, status TEXT DEFAULT 'active')"));
        QVERIFY(execute(
            "CREATE TABLE ACCOUNTS ("
            "idEmployee INTEGER NOT NULL, userName TEXT NOT NULL, passWord TEXT NOT NULL)"));
        QVERIFY(execute(
            "CREATE TABLE SHIFT ("
            "IdShift INTEGER PRIMARY KEY, idEmployee INTEGER NOT NULL, "
            "workDate TEXT NOT NULL, startTime TEXT NOT NULL, endTime TEXT NOT NULL, "
            "status INTEGER DEFAULT 0, isHoliday INTEGER DEFAULT 0)"));
        QVERIFY(execute(
            "INSERT INTO PROFILES (idEmployee, role, isFixed) VALUES "
            "(1001, 'Cashier', 1), (1002, 'HallStaff', 0), "
            "(1003, 'Cashier', 0), (1004, 'Cashier', 0), "
            "(1005, 'KitchenAssistant', 0), "
            "(2001, 'Manager', 0), (2002, 'Admin', 0)"));
        QSqlQuery account(database());
        account.prepare(
            "INSERT INTO ACCOUNTS (idEmployee, userName, passWord) "
            "VALUES (1001, 'fixed.user', :password)");
        account.bindValue(":password", Security::hashPassword("secret"));
        QVERIFY(account.exec());

        account.prepare(
            "INSERT INTO ACCOUNTS (idEmployee, userName, passWord) "
            "VALUES (1002, 'initial.user', :password)");
        account.bindValue(":password", Security::markInitialPasswordHash(
            Security::hashPassword("initial-secret")));
        QVERIFY(account.exec());
    }

    void init()
    {
        Config::setOpenHour(7);
        Config::setCloseHour(22);
        Config::setRoles({
            {"Cashier", {1, 6}},
            {"HallStaff", {1, 6}},
            {"KitchenAssitant", {1, 6}},
            {"Manager", {1, 6}}});
        QVERIFY(execute("DROP TRIGGER IF EXISTS reject_evening"));
        QVERIFY(execute("DELETE FROM SHIFT"));
        QVERIFY(execute("DELETE FROM NOTIFICATION"));
        QVERIFY(execute("DELETE FROM LEAVE_REQUEST"));
        QVERIFY(execute("DELETE FROM SHIFT_AUDIT"));
        QVERIFY(execute("DELETE FROM SHIFT_CARRY_FORWARD"));
        QVERIFY(execute("UPDATE PROFILES SET status = 'active'"));
    }

    void employeePayTypeIsStored()
    {
        QCOMPARE(scalar("SELECT isFixed FROM PROFILES WHERE idEmployee = 1001"), 1);
        QCOMPARE(scalar("SELECT isFixed FROM PROFILES WHERE idEmployee = 1002"), 0);
        QCOMPARE(scheduleLayoutModeForPayType(true),
                 EmployeeScheduleLayoutMode::FullTimeSchedule);
        QCOMPARE(scheduleLayoutModeForPayType(false),
                 EmployeeScheduleLayoutMode::PartTimeHourly);
    }

    void loginPreservesEmployeePayType()
    {
        Login_Model login;
        std::unique_ptr<User> user(login.verifyLogin("fixed.user", "secret"));
        QVERIFY(user != nullptr);
        QVERIFY(user->getIsFixedSalary());
    }

    void initialPasswordRequiresChangeAndIsClearedAfterUpdate()
    {
        Login_Model login;
        bool mustChangePassword = false;
        std::unique_ptr<User> user(login.verifyLogin(
            "initial.user", "initial-secret", &mustChangePassword));
        QVERIFY(user != nullptr);
        QVERIFY(mustChangePassword);

        Change_password changePassword;
        // Rejects weak passwords
        QCOMPARE(changePassword.updatePassword(
                     user->getIdEmployee(), "initial-secret", "weak"),
                 PasswordChangeResult::NEW_PASSWORD_TOO_WEAK);
        QCOMPARE(changePassword.updatePassword(
                     user->getIdEmployee(), "initial-secret", "Pass$123!"),
                 PasswordChangeResult::NEW_PASSWORD_TOO_WEAK);
        QCOMPARE(changePassword.updatePassword(
                     user->getIdEmployee(), "initial-secret", "password123!"),
                 PasswordChangeResult::NEW_PASSWORD_TOO_WEAK);
        QCOMPARE(changePassword.updatePassword(
                     user->getIdEmployee(), "initial-secret", "PASSWORD123!"),
                 PasswordChangeResult::NEW_PASSWORD_TOO_WEAK);
        QCOMPARE(changePassword.updatePassword(
                     user->getIdEmployee(), "initial-secret", "Password!"),
                 PasswordChangeResult::NEW_PASSWORD_TOO_WEAK);

        // Accepts valid strong password without '$'
        QCOMPARE(changePassword.updatePassword(
                     user->getIdEmployee(), "initial-secret", "Changed@123"),
                 PasswordChangeResult::SUCCESS);

        QSqlQuery query(database());
        query.prepare("SELECT passWord FROM ACCOUNTS WHERE userName = 'initial.user'");
        QVERIFY(query.exec());
        QVERIFY(query.next());
        QVERIFY(!Security::isInitialPasswordHash(query.value(0).toString()));
        QCOMPARE(query.value(0).toString(), Security::hashPassword("Changed@123"));

        mustChangePassword = true;
        std::unique_ptr<User> changedUser(login.verifyLogin(
            "initial.user", "Changed@123", &mustChangePassword));
        QVERIFY(changedUser != nullptr);
        QVERIFY(!mustChangePassword);
    }

    void fullTimeGridMapsDatabaseStatuses()
    {
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES "
            "(1001, '2026-08-03', '07:00', '12:00', 0), "
            "(1001, '2026-08-04', '12:00', '17:00', 1), "
            "(1001, '2026-08-05', '17:00', '22:00', -1)"));

        Schedule_Model model;
        FullTimeScheduleGrid grid =
            model.getFullTimeScheduleGrid(1001, weekStart);

        QCOMPARE(grid[0][0], FullTimeShiftStatus::Pending);
        QCOMPARE(grid[1][1], FullTimeShiftStatus::Approved);
        QCOMPARE(grid[2][2], FullTimeShiftStatus::Declined);
        QCOMPARE(grid[6][0], FullTimeShiftStatus::Available);
    }

    void fullTimeGridDerivesStaffingShortage()
    {
        Config::setRoles({{"Cashier", {2, 6}}});
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) "
            "VALUES (1003, '2026-08-03', '07:00', '12:00', 1), "
            "(1003, '2026-08-03', '12:00', '17:00', 1), "
            "(1004, '2026-08-03', '12:00', '17:00', 1)"));

        Schedule_Model model;
        FullTimeScheduleGrid grid =
            model.getFullTimeScheduleGrid(1001, weekStart);
        QCOMPARE(grid[0][0], FullTimeShiftStatus::StaffShortage);
        QCOMPARE(grid[0][1], FullTimeShiftStatus::StaffSufficient);
    }

    void salaryUsesStoredPayType()
    {
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status, isHoliday) VALUES "
            "(1001, '2026-08-03', '07:00', '12:00', 1, 0), "
            "(1002, '2026-08-03', '07:00', '12:00', 1, 0)"));

        SalaryData fixed = Salary_Model::getSalarySummary(
            1001, "Cashier", 100.0, 8, 2026);
        SalaryData hourly = Salary_Model::getSalarySummary(
            1002, "HallStaff", 100.0, 8, 2026);

        QCOMPARE(fixed.normalHours, 1);
        QCOMPARE(hourly.normalHours, 5);
    }

    void pendingRowsAreAtomicallyReplacedAndApprovedRowsRemain()
    {
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES "
            "(1001, '2026-08-04', '12:00', '17:00', 1)"));

        Schedule_Model model;
        QList<StaffShiftRegistration> registrations = {
            {weekStart, QTime(7, 0), QTime(12, 0)},
            {weekStart.addDays(2), QTime(17, 0), QTime(22, 0)}};
        QVERIFY(model.replacePendingShiftsForWeek(1001, weekStart,
                                                   registrations));
        QCOMPARE(scalar("SELECT COUNT(*) FROM SHIFT WHERE status = 0"), 2);

        registrations.removeLast();
        QVERIFY(model.replacePendingShiftsForWeek(1001, weekStart,
                                                   registrations));
        QCOMPARE(scalar("SELECT COUNT(*) FROM SHIFT WHERE status = 0"), 1);
        QCOMPARE(scalar("SELECT COUNT(*) FROM SHIFT WHERE status = 1"), 1);

        QVERIFY(model.replacePendingShiftsForWeek(1001, weekStart, {}));
        QCOMPARE(scalar("SELECT COUNT(*) FROM SHIFT WHERE status = 0"), 0);
        QCOMPARE(scalar("SELECT COUNT(*) FROM SHIFT WHERE status = 1"), 1);
    }

    void configuredShopHoursDefineCanonicalShiftBoundaries()
    {
        Config::setOpenHour(8);
        Config::setCloseHour(23);

        QCOMPARE(Config::getShiftStartTime(0), QTime(8, 0));
        QCOMPARE(Config::getShiftEndTime(0), QTime(12, 0));
        QCOMPARE(Config::getShiftTimeLabel(0), QString("08:00 - 12:00"));
        QCOMPARE(Config::getShiftTimeLabel(1), QString("12:00 - 17:00"));
        QCOMPARE(Config::getShiftStartTime(2), QTime(17, 0));
        QCOMPARE(Config::getShiftEndTime(2), QTime(23, 0));
        QCOMPARE(Config::getShiftTimeLabel(2), QString("17:00 - 23:00"));

        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES "
            "(1001, '2026-08-03', '08:00', '12:00', 0), "
            "(1001, '2026-08-04', '17:00', '23:00', 1)"));

        Schedule_Model model;
        const FullTimeScheduleGrid grid =
            model.getFullTimeScheduleGrid(1001, weekStart);
        QCOMPARE(grid[0][0], FullTimeShiftStatus::Pending);
        QCOMPARE(grid[1][2], FullTimeShiftStatus::Approved);
    }

    void scheduleWeeksAlwaysUseMondayRegardlessOfOpenDay()
    {
        const QDate openTuesday(2026, 8, 11);
        QCOMPARE(Config::getStartOfCurrentWeek(openTuesday),
                 QDate(2026, 8, 10));
        QCOMPARE(Config::getStartOfNextWeek(openTuesday),
                 QDate(2026, 8, 17));

        Config::setDayOpenRegisShift(Qt::Wednesday);
        QCOMPARE(Config::getStartOfCurrentWeek(QDate(2026, 8, 12)),
                 QDate(2026, 8, 10));
        QCOMPARE(Config::getStartOfNextWeek(QDate(2026, 8, 12)),
                 QDate(2026, 8, 17));
        Config::setDayOpenRegisShift(Qt::Tuesday);
    }

    void carryForwardCopiesOnlyApprovedShiftsWithExactTimes()
    {
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES "
            "(1001, '2026-07-27', '07:30', '11:45', 1), "
            "(1001, '2026-07-28', '12:00', '17:00', 0), "
            "(1001, '2026-07-29', '17:00', '22:00', -1), "
            "(1001, '2026-07-30', '07:00', '12:00', -2)"));

        Schedule_Model model;
        QStringList errors;
        QVERIFY2(model.ensurePendingCarryForwardForWeek(weekStart, &errors),
                 qPrintable(errors.join(" | ")));

        QCOMPARE(scalar(
            "SELECT COUNT(*) FROM SHIFT WHERE idEmployee = 1001 "
            "AND workDate BETWEEN '2026-08-03' AND '2026-08-09' AND status = 0"), 1);
        QSqlQuery copied(database());
        QVERIFY(copied.exec(
            "SELECT workDate, startTime, endTime FROM SHIFT "
            "WHERE idEmployee = 1001 AND workDate = '2026-08-03' AND status = 0"));
        QVERIFY(copied.next());
        QCOMPARE(copied.value(0).toDate(), weekStart);
        QCOMPARE(copied.value(1).toTime(), QTime(7, 30));
        QCOMPARE(copied.value(2).toTime(), QTime(11, 45));
        QCOMPARE(scalar("SELECT COUNT(*) FROM NOTIFICATION"), 0);
        QCOMPARE(scalar(
            "SELECT COUNT(*) FROM SHIFT_CARRY_FORWARD "
            "WHERE idEmployee = 1001 AND targetWeekStart = '2026-08-03'"), 1);
    }

    void carryForwardSkipsExistingConflictsAndApprovedLeave()
    {
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES "
            "(1001, '2026-07-27', '07:00', '12:00', 1), "
            "(1002, '2026-07-28', '12:00', '17:00', 1), "
            "(1003, '2026-07-29', '17:00', '22:00', 1), "
            "(1001, '2026-08-03', '10:00', '14:00', 0)"));
        QVERIFY(execute(
            "INSERT INTO LEAVE_REQUEST "
            "(idEmployee, leaveDate, reason, status, requestedAt) VALUES "
            "(1002, '2026-08-04', 'Approved leave', 'Approved', '2026-07-25T00:00:00Z')"));

        Schedule_Model model;
        QStringList errors;
        QVERIFY2(model.ensurePendingCarryForwardForWeek(weekStart, &errors),
                 qPrintable(errors.join(" | ")));

        QCOMPARE(scalar(
            "SELECT COUNT(*) FROM SHIFT WHERE idEmployee = 1001 "
            "AND workDate = '2026-08-03' AND status = 0"), 1);
        QCOMPARE(scalar(
            "SELECT COUNT(*) FROM SHIFT WHERE idEmployee = 1002 "
            "AND workDate = '2026-08-04' AND status = 0"), 0);
        QCOMPARE(scalar(
            "SELECT COUNT(*) FROM SHIFT WHERE idEmployee = 1003 "
            "AND workDate = '2026-08-05' AND startTime = '17:00:00.000' "
            "AND endTime = '22:00:00.000' AND status = 0"), 1);
    }

    void carryForwardMarkerPreventsDeletedRowsFromReturning()
    {
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) "
            "VALUES (1001, '2026-07-27', '07:00', '12:00', 1)"));

        Schedule_Model model;
        QVERIFY(model.ensurePendingCarryForwardForWeek(weekStart));
        QVERIFY(execute(
            "DELETE FROM SHIFT WHERE idEmployee = 1001 "
            "AND workDate = '2026-08-03' AND status = 0"));
        QVERIFY(model.ensurePendingCarryForwardForWeek(weekStart));

        QCOMPARE(scalar(
            "SELECT COUNT(*) FROM SHIFT WHERE idEmployee = 1001 "
            "AND workDate = '2026-08-03' AND status = 0"), 0);
        QCOMPARE(scalar(
            "SELECT COUNT(*) FROM SHIFT_CARRY_FORWARD "
            "WHERE idEmployee = 1001 AND targetWeekStart = '2026-08-03'"), 1);
    }

    void carryForwardFailureRollsBackRowsAndMarker()
    {
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) "
            "VALUES (1001, '2026-07-27', '17:00', '22:00', 1)"));
        QVERIFY(execute(
            "CREATE TRIGGER reject_evening BEFORE INSERT ON SHIFT "
            "WHEN time(NEW.startTime) = '17:00:00' "
            "BEGIN SELECT RAISE(ABORT, 'carry insert failure'); END"));

        Schedule_Model model;
        QStringList errors;
        QVERIFY(!model.ensurePendingCarryForwardForWeek(weekStart, &errors));
        QVERIFY(!errors.isEmpty());
        QCOMPARE(scalar(
            "SELECT COUNT(*) FROM SHIFT WHERE workDate BETWEEN '2026-08-03' AND '2026-08-09'"), 0);
        QCOMPARE(scalar("SELECT COUNT(*) FROM SHIFT_CARRY_FORWARD"), 0);
    }

    void employeeChooserFiltersSortsAndDisablesConflicts()
    {
        QList<EligibleEmployeeInfo> employees = {
            {1002, "Binh", "HallStaff", false, true, {}},
            {1001, "An", "Cashier", true, true, {}},
            {1003, "Cuong", "HallStaff", false, false, "Shift conflict"}};
        ManagerEmployeeChooser_Dialog chooser(
            employees, QTime(7, 0), QTime(12, 0));

        QLineEdit *search = chooser.findChild<QLineEdit *>("employeeSearch");
        QComboBox *roleFilter = chooser.findChild<QComboBox *>("employeeRoleFilter");
        QComboBox *sort = chooser.findChild<QComboBox *>("employeeSort");
        QTableWidget *table = chooser.findChild<QTableWidget *>("employeeChooserTable");
        QVERIFY(search);
        QVERIFY(roleFilter);
        QVERIFY(sort);
        QVERIFY(table);
        QCOMPARE(table->rowCount(), 3);
        QCOMPARE(table->item(0, 0)->text(), QString("NV-1001"));

        search->setText("1002");
        QCOMPARE(table->rowCount(), 1);
        QCOMPARE(table->item(0, 1)->text(), QString("Binh"));
        search->clear();

        roleFilter->setCurrentIndex(roleFilter->findData("HallStaff"));
        QCOMPARE(table->rowCount(), 2);
        roleFilter->setCurrentIndex(0);

        sort->setCurrentIndex(sort->findData("id_desc"));
        QCOMPARE(table->item(0, 0)->text(), QString("NV-1003"));
        QVERIFY(!(table->item(0, 0)->flags() & Qt::ItemIsUserCheckable));
        QCOMPARE(table->item(0, 0)->toolTip(), QString("Shift conflict"));
    }

    void employeeChooserExcludesManagersAndLocksFixedTimes()
    {
        QList<EligibleEmployeeInfo> employees = {
            {1001, "An", "Cashier", true, true, {}},
            {1002, "Binh", "HallStaff", false, true, {}},
            {1003, "Minh", "Manager", false, true, {}}};
        ManagerEmployeeChooser_Dialog chooser(
            employees, QTime(12, 0), QTime(17, 0));
        QTableWidget *table = chooser.findChild<QTableWidget *>("employeeChooserTable");
        QVERIFY(table);

        auto rowForId = [table](int employeeId) {
            const QString expected = QString("NV-%1").arg(employeeId);
            for (int row = 0; row < table->rowCount(); ++row)
                if (table->item(row, 0) && table->item(row, 0)->text() == expected)
                    return row;
            return -1;
        };
        const int fixedRow = rowForId(1001);
        const int flexibleRow = rowForId(1002);
        const int managerRow = rowForId(1003);
        QVERIFY(fixedRow >= 0);
        QVERIFY(flexibleRow >= 0);
        QCOMPARE(managerRow, -1);
        QCOMPARE(table->rowCount(), 2);
        QComboBox *roleFilter =
            chooser.findChild<QComboBox *>("employeeRoleFilter");
        QVERIFY(roleFilter);
        QCOMPARE(roleFilter->findData("Manager"), -1);

        QTimeEdit *fixedStart = qobject_cast<QTimeEdit *>(table->cellWidget(fixedRow, 3));
        QTimeEdit *fixedEnd = qobject_cast<QTimeEdit *>(table->cellWidget(fixedRow, 4));
        QTimeEdit *flexibleStart = qobject_cast<QTimeEdit *>(table->cellWidget(flexibleRow, 3));
        QTimeEdit *flexibleEnd = qobject_cast<QTimeEdit *>(table->cellWidget(flexibleRow, 4));
        QVERIFY(fixedStart && fixedEnd && flexibleStart && flexibleEnd);
        QCOMPARE(fixedStart->buttonSymbols(), QAbstractSpinBox::NoButtons);
        QCOMPARE(fixedEnd->buttonSymbols(), QAbstractSpinBox::NoButtons);
        QCOMPARE(flexibleStart->buttonSymbols(), QAbstractSpinBox::NoButtons);
        QCOMPARE(flexibleEnd->buttonSymbols(), QAbstractSpinBox::NoButtons);
        QVERIFY(!fixedStart->isEnabled());
        QVERIFY(!fixedEnd->isEnabled());
        QCOMPARE(fixedStart->time(), QTime(12, 0));
        QCOMPARE(fixedEnd->time(), QTime(17, 0));
        QVERIFY(flexibleStart->isEnabled());
        QVERIFY(flexibleEnd->isEnabled());

        flexibleStart->setTime(QTime(13, 0));
        flexibleEnd->setTime(QTime(16, 30));
        table->item(fixedRow, 0)->setCheckState(Qt::Checked);
        table->item(flexibleRow, 0)->setCheckState(Qt::Checked);
        const QList<ManagerEmployeeSelection> selected = chooser.selections();
        QCOMPARE(selected.size(), 2);
        auto flexible = std::find_if(
            selected.cbegin(), selected.cend(),
            [](const ManagerEmployeeSelection &selection) {
                return selection.employeeId == 1002;
            });
        QVERIFY(flexible != selected.cend());
        QCOMPARE(flexible->startTime, QTime(13, 0));
        QCOMPARE(flexible->endTime, QTime(16, 30));
    }

    void eligibleEmployeesExcludeManagers()
    {
        Schedule_Model model;
        const QList<EligibleEmployeeInfo> employees =
            model.getEligibleEmployees(weekStart, QTime(7, 0), QTime(12, 0));
        const bool containsManager = std::any_of(
            employees.cbegin(), employees.cend(),
            [](const EligibleEmployeeInfo &employee) {
                return employee.role.compare("Manager", Qt::CaseInsensitive) == 0 ||
                       employee.role.compare("Manage", Qt::CaseInsensitive) == 0;
            });
        QVERIFY(!containsManager);
    }

    void approvedOverlapIsRejected()
    {
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) "
            "VALUES (1001, '2026-08-03', '07:00', '12:00', 1)"));

        Schedule_Model model;
        QList<StaffShiftRegistration> registrations = {
            {weekStart, QTime(7, 0), QTime(12, 0)}};
        QVERIFY(!model.replacePendingShiftsForWeek(1001, weekStart,
                                                    registrations));
        QCOMPARE(scalar("SELECT COUNT(*) FROM SHIFT WHERE status = 1"), 1);
        QCOMPARE(scalar("SELECT COUNT(*) FROM SHIFT WHERE status = 0"), 0);
    }

    void failedInsertRollsBackPendingDeletions()
    {
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES "
            "(1001, '2026-08-03', '07:00', '12:00', 0), "
            "(1001, '2026-08-04', '12:00', '17:00', 0)"));
        QVERIFY(execute(
            "CREATE TRIGGER reject_evening BEFORE INSERT ON SHIFT "
            "WHEN time(NEW.startTime) = '17:00:00' "
            "BEGIN SELECT RAISE(ABORT, 'test insert failure'); END"));

        Schedule_Model model;
        QList<StaffShiftRegistration> registrations = {
            {weekStart, QTime(7, 0), QTime(12, 0)},
            {weekStart.addDays(2), QTime(17, 0), QTime(22, 0)}};
        QVERIFY(!model.replacePendingShiftsForWeek(1001, weekStart,
                                                    registrations));
        QCOMPARE(scalar("SELECT COUNT(*) FROM SHIFT WHERE status = 0"), 2);
        QCOMPARE(scalar(
            "SELECT COUNT(*) FROM SHIFT WHERE workDate = '2026-08-04'"), 1);
    }

    void managerOverrideAddsAndCancelsWithHistory()
    {
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) "
            "VALUES (1001, '2026-08-03', '07:00', '12:00', 1)"));

        Schedule_Model model;
        QList<ManagerScheduleChange> changes = {
            {ManagerScheduleChangeType::Add, 0, 1002, {}, {}, weekStart,
             QTime(12, 0), QTime(15, 0), "Manual coverage"},
            {ManagerScheduleChangeType::Cancel, 1, 1001, {}, {}, {}, {}, {},
             "Approved employee replaced"}};
        QStringList errors;
        QVERIFY2(model.applyManagerScheduleChanges(changes, &errors), qPrintable(errors.join(" | ")));
        QCOMPARE(scalar("SELECT COUNT(*) FROM SHIFT WHERE status = 1"), 1);
        QCOMPARE(scalar("SELECT COUNT(*) FROM SHIFT WHERE status = -2"), 1);
        QCOMPARE(scalar("SELECT COUNT(*) FROM SHIFT_AUDIT"), 2);
    }

    void managerOverrideRejectsOverlapAtomically()
    {
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) "
            "VALUES (1001, '2026-08-03', '07:00', '12:00', 1)"));

        Schedule_Model model;
        QList<ManagerScheduleChange> changes = {
            {ManagerScheduleChangeType::Add, 0, 1001, {}, {}, weekStart,
             QTime(10, 0), QTime(14, 0), "Conflicting manual edit"}};
        QStringList errors;
        QVERIFY(!model.applyManagerScheduleChanges(changes, &errors));
        QVERIFY(!errors.isEmpty());
        QCOMPARE(scalar("SELECT COUNT(*) FROM SHIFT"), 1);
    }

    void managerOverrideRejectsManagerTargets()
    {
        Schedule_Model model;
        QList<ManagerScheduleChange> changes = {
            {ManagerScheduleChangeType::Add, 0, 2001, "Manager", "Manager", weekStart,
             QTime(7, 0), QTime(12, 0), "Invalid manager assignment"}};
        QStringList errors;

        QVERIFY(!model.applyManagerScheduleChanges(changes, &errors));
        QVERIFY(!errors.isEmpty());
        QCOMPARE(scalar("SELECT COUNT(*) FROM SHIFT"), 0);
    }

    void eligibleEmployeesExplainConflict()
    {
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) "
            "VALUES (1001, '2026-08-03', '07:00', '12:00', 1)"));

        Schedule_Model model;
        QList<EligibleEmployeeInfo> employees =
            model.getEligibleEmployees(weekStart, QTime(10, 0), QTime(14, 0));
        auto it = std::find_if(employees.begin(), employees.end(),
                               [](const EligibleEmployeeInfo &info) {
                                   return info.employeeId == 1001;
                               });
        QVERIFY(it != employees.end());
        QVERIFY(!it->eligible);
        QVERIFY(!it->reason.isEmpty());
    }

    void eligibleEmployeesDisableInactiveProfiles()
    {
        QVERIFY(execute(
            "UPDATE PROFILES SET status = 'suspended' WHERE idEmployee = 1002"));
        Schedule_Model model;
        const QList<EligibleEmployeeInfo> employees =
            model.getEligibleEmployees(weekStart, QTime(7, 0), QTime(12, 0));
        auto it = std::find_if(
            employees.cbegin(), employees.cend(),
            [](const EligibleEmployeeInfo &info) {
                return info.employeeId == 1002;
            });
        QVERIFY(it != employees.cend());
        QVERIFY(!it->eligible);
        QVERIFY(!it->reason.isEmpty());
        QVERIFY(execute(
            "UPDATE PROFILES SET status = 'active' WHERE idEmployee = 1002"));
    }

    void automaticSchedulePreviewDoesNotWriteDatabase()
    {
        Config::setRoles({{"Cashier", {1, 6}}});
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) "
            "VALUES (1001, '2026-08-03', '07:00', '12:00', 0)"));

        Schedule_Model model;
        AutoSchedulePreview preview = model.previewGeneratedSchedule(weekStart);

        QCOMPARE(scalar("SELECT status FROM SHIFT WHERE IdShift = 1"), 0);
        QCOMPARE(preview.changes.size(), 1);
        QCOMPARE(static_cast<int>(preview.changes.first().type),
                 static_cast<int>(ManagerScheduleChangeType::Approve));
        QVERIFY(preview.approvedCount + preview.declinedCount ==
                preview.changes.size());
    }

    void automaticPreviewCarriesOptimizerAssignedTime()
    {
        Config::setRoles({{"HallStaff", {1, 6}}});
        Config::setMinimumDaysWorkPerWeek_PT(1);
        Config::setMinimumHourWorkPerDay_PT(1);
        Config::setMaximumHourWorkPerDay_PT(8);
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) "
            "VALUES (1002, '2026-08-03', '10:00', '20:00', 0)"));

        Schedule_Model model;
        AutoSchedulePreview preview = model.previewGeneratedSchedule(weekStart);

        QCOMPARE(preview.changes.size(), 1);
        QCOMPARE(static_cast<int>(preview.changes.first().type),
                 static_cast<int>(ManagerScheduleChangeType::Approve));
        QCOMPARE(preview.changes.first().startTime, QTime(10, 0));
        QCOMPARE(preview.changes.first().endTime, QTime(12, 0));
        QCOMPARE(scalar("SELECT status FROM SHIFT WHERE IdShift = 1"), 0);
    }

    void automaticPreviewParsesDatabaseTimesWithMilliseconds()
    {
        Config::setRoles({{"HallStaff", {1, 6}}});
        Config::setMinimumDaysWorkPerWeek_PT(1);
        Config::setMinimumHourWorkPerDay_PT(1);
        Config::setMaximumHourWorkPerDay_PT(8);
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) "
            "VALUES (1002, '2026-08-03', '10:00:00.000', '20:00:00.000', 0)"));

        Schedule_Model model;
        AutoSchedulePreview preview = model.previewGeneratedSchedule(weekStart);

        QCOMPARE(preview.changes.size(), 1);
        QCOMPARE(static_cast<int>(preview.changes.first().type),
                 static_cast<int>(ManagerScheduleChangeType::Approve));
        QCOMPARE(preview.changes.first().startTime, QTime(10, 0));
        QCOMPARE(preview.changes.first().endTime, QTime(12, 0));
    }

    void approvingScheduleChangePersistsAssignedTime()
    {
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) "
            "VALUES (1002, '2026-08-03', '10:00', '20:00', 0)"));

        Schedule_Model model;
        QList<ManagerScheduleChange> changes = {
            {ManagerScheduleChangeType::Approve, 1, 1002, "Employee", "HallStaff",
             weekStart, QTime(10, 0), QTime(12, 0), "Automatic assignment"}};
        QStringList errors;
        QVERIFY2(model.applyManagerScheduleChanges(changes, &errors),
                 qPrintable(errors.join(" | ")));
        QCOMPARE(scalar("SELECT status FROM SHIFT WHERE IdShift = 1"), 1);

        QSqlQuery query(database());
        QVERIFY(query.exec("SELECT startTime, endTime FROM SHIFT WHERE IdShift = 1"));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toTime(), QTime(10, 0));
        QCOMPARE(query.value(1).toTime(), QTime(12, 0));
    }

    void draftValidationRejectsConflictingShiftActions()
    {
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) "
            "VALUES (1001, '2026-08-03', '07:00', '12:00', 0)"));

        Schedule_Model model;
        QList<ManagerScheduleChange> changes = {
            {ManagerScheduleChangeType::Approve, 1, 1001, "Employee", "Cashier",
             weekStart, QTime(7, 0), QTime(12, 0), "Approve"},
            {ManagerScheduleChangeType::Decline, 1, 1001, "Employee", "Cashier",
             weekStart, QTime(7, 0), QTime(12, 0), "Decline"}};
        QStringList errors = model.validateManagerScheduleChanges(changes);

        QVERIFY(!errors.isEmpty());
        QCOMPARE(scalar("SELECT status FROM SHIFT WHERE IdShift = 1"), 0);
    }

    void managerDecisionCreatesStaffNotification()
    {
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) "
            "VALUES (1002, '2026-08-03', '07:00', '12:00', 0)"));

        Schedule_Model schedule;
        QList<ManagerScheduleChange> changes = {
            {ManagerScheduleChangeType::Approve, 1, 1002, "Employee", "HallStaff",
             weekStart, QTime(7, 0), QTime(12, 0), "Approved for coverage"}};
        QStringList errors;
        QVERIFY2(schedule.applyManagerScheduleChanges(changes, &errors),
                 qPrintable(errors.join(" | ")));

        Notification_Model notifications;
        const QList<NotificationInfo> staffNotifications =
            notifications.getNotifications(1002);
        QCOMPARE(staffNotifications.size(), 1);
        QCOMPARE(staffNotifications.first().type, QString("SHIFT_APPROVED"));
        QCOMPARE(notifications.getUnreadCount(1002), 1);
        QVERIFY(notifications.markAsRead(staffNotifications.first().id, 1002));
        QCOMPARE(notifications.getUnreadCount(1002), 0);
    }

    void newPendingShiftNotifiesManagerOnlyOnce()
    {
        Schedule_Model schedule;
        const QList<StaffShiftRegistration> registrations = {
            {weekStart, QTime(7, 0), QTime(12, 0)}};
        QVERIFY(schedule.replacePendingShiftsForWeek(1002, weekStart, registrations));
        QVERIFY(schedule.replacePendingShiftsForWeek(1002, weekStart, registrations));

        Notification_Model notifications;
        const QList<NotificationInfo> managerNotifications =
            notifications.getNotifications(2001);
        QCOMPARE(managerNotifications.size(), 1);
        QCOMPARE(managerNotifications.first().type, QString("SHIFT_SUBMITTED"));
    }

    void readNotificationsCanBeDeletedAndUnreadStayFirst()
    {
        QSqlDatabase db = database();
        QVERIFY(Notification_Model::create(db, 1002, "SHIFT_APPROVED",
                                           "Older", "Read this first"));
        QVERIFY(Notification_Model::create(db, 1002, "SHIFT_DECLINED",
                                           "Newer", "Keep this unread"));

        Notification_Model notifications;
        const QList<NotificationInfo> created = notifications.getNotifications(1002);
        QCOMPARE(created.size(), 2);
        QVERIFY(notifications.markAsRead(created.first().id, 1002));

        const QList<NotificationInfo> ordered = notifications.getNotifications(1002);
        QCOMPARE(ordered.size(), 2);
        QCOMPARE(ordered.first().status, QString("Unread"));
        QVERIFY(notifications.deleteAllRead(1002));

        const QList<NotificationInfo> remaining = notifications.getNotifications(1002);
        QCOMPARE(remaining.size(), 1);
        QCOMPARE(remaining.first().status, QString("Unread"));
    }

    void staffingWarningsArePrioritizedAndDeduplicated()
    {
        Config::setRoles({{"Cashier", {4, 6}}});
        const QDate currentWeek = Config::getStartOfCurrentWeek(QDate::currentDate());
        const QDate warningWeek = currentWeek.addDays(7);
        const QDate futureDate = warningWeek;
        QVERIFY(execute(QString(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) "
            "VALUES (1001, '%1', '07:00', '12:00', 1)")
            .arg(futureDate.toString(Qt::ISODate))));

        Schedule_Model schedule;
        schedule.publishStaffingWarningNotifications(warningWeek);
        Notification_Model notifications;
        const QList<NotificationInfo> firstLoad = notifications.getNotifications(2001);
        QVERIFY(!firstLoad.isEmpty());
        QCOMPARE(firstLoad.first().type, QString("STAFFING_SHORTAGE"));
        QCOMPARE(firstLoad.first().priority, 100);
        const int countAfterFirstPublish = firstLoad.size();

        schedule.publishStaffingWarningNotifications(warningWeek);
        QCOMPARE(notifications.getNotifications(2001).size(), countAfterFirstPublish);
    }

    void roleAwareCountsRequireEveryOperationalRole()
    {
        Config::setRoles({
            {"Cashier", {1, 6}},
            {"HallStaff", {1, 6}},
            {"KitchenAssitant", {1, 6}},
            {"Manager", {1, 6}}});
        QVERIFY(Config::isOperationalRole("Cashier"));
        QVERIFY(!Config::isOperationalRole("Manager"));
        QVERIFY(!Config::isOperationalRole("Admin"));
        QCOMPARE(Config::getOperationalRoles().size(), 3);
        QCOMPARE(Config::getMinStaffForRole("KitchenAssistant"), 1);

        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES "
            "(1001, '2026-08-03', '07:00', '12:00', 1), "
            "(1003, '2026-08-03', '07:00', '12:00', 1), "
            "(1004, '2026-08-03', '07:00', '12:00', 1), "
            "(2001, '2026-08-03', '07:00', '12:00', 1), "
            "(2002, '2026-08-03', '07:00', '12:00', 1)"));

        Schedule_Model model;
        BlockCounts wrongMix = model.getAssignBlockCounts(weekStart)[0][0];
        QCOMPARE(wrongMix.required, 3);
        QCOMPARE(wrongMix.accepted, 3);
        QCOMPARE(wrongMix.byRole.value("Cashier").accepted, 3);
        QCOMPARE(wrongMix.byRole.value("HallStaff").accepted, 0);
        QCOMPARE(wrongMix.byRole.value("KitchenAssistant").accepted, 0);
        QCOMPARE(wrongMix.missingSlots(), 2);
        QVERIFY(wrongMix.hasShortage());
        QVERIFY(!wrongMix.byRole.contains("Manager"));
        QVERIFY(!wrongMix.byRole.contains("Admin"));

        QVERIFY(execute("DELETE FROM SHIFT"));
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES "
            "(1001, '2026-08-03', '07:00', '12:00', 1), "
            "(1002, '2026-08-03', '07:00', '12:00', 1), "
            "(1005, '2026-08-03', '07:00', '12:00', 1)"));

        BlockCounts correctMix = model.getAssignBlockCounts(weekStart)[0][0];
        QCOMPARE(correctMix.accepted, 3);
        QCOMPARE(correctMix.missingSlots(), 0);
        QVERIFY(!correctMix.hasShortage());
    }

    void roleAwareCountsPreserveStatusBreakdownAndDraftDeltas()
    {
        Config::setRoles({
            {"Cashier", {1, 6}},
            {"HallStaff", {1, 6}},
            {"KitchenAssistant", {1, 6}}});
        QVERIFY(execute(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES "
            "(1001, '2026-08-03', '07:00', '12:00', 0), "
            "(1002, '2026-08-03', '07:00', '12:00', 1), "
            "(1005, '2026-08-03', '07:00', '12:00', -1), "
            "(1003, '2026-08-03', '07:00', '12:00', -2)"));

        Schedule_Model model;
        BlockCounts counts = model.getAssignBlockCounts(weekStart)[0][0];
        QCOMPARE(counts.pending, 1);
        QCOMPARE(counts.accepted, 1);
        QCOMPARE(counts.declined, 1);
        QCOMPARE(counts.cancelled, 1);
        QCOMPARE(counts.byRole.value("Cashier").pending, 1);
        QCOMPARE(counts.byRole.value("Cashier").cancelled, 1);
        QCOMPARE(counts.byRole.value("HallStaff").accepted, 1);
        QCOMPARE(counts.byRole.value("KitchenAssistant").declined, 1);

        counts.adjustStatus("Cashier", 0, -1);
        counts.adjustStatus("Cashier", 1, 1);
        QCOMPARE(counts.pending, 0);
        QCOMPARE(counts.accepted, 2);
        QCOMPARE(counts.byRole.value("Cashier").accepted, 1);
        QCOMPARE(counts.missingSlots(), 1);

        counts.adjustStatus("KitchenAssistant", 1, 1);
        QCOMPARE(counts.missingSlots(), 0);
        QVERIFY(!counts.hasShortage());
        counts.adjustStatus("HallStaff", 1, -1);
        counts.adjustStatus("HallStaff", -2, 1);
        QCOMPARE(counts.cancelled, 2);
        QCOMPARE(counts.missingByRole().value("HallStaff"), 1);
    }

    void staffingWarningKeyTracksRoleDeficitMix()
    {
        Config::setRoles({
            {"Cashier", {1, 6}},
            {"HallStaff", {1, 6}}});
        const QDate warningWeek =
            Config::getStartOfCurrentWeek(QDate::currentDate()).addDays(7);
        const QString date = warningWeek.toString(Qt::ISODate);
        QVERIFY(execute(QString(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES "
            "(1001, '%1', '07:00', '12:00', 1), "
            "(1003, '%1', '07:00', '12:00', 1)").arg(date)));

        Schedule_Model schedule;
        Notification_Model notifications;
        schedule.publishStaffingWarningNotifications(warningWeek);
        const QList<NotificationInfo> first = notifications.getNotifications(2001);
        auto hallShortage = std::find_if(
            first.cbegin(), first.cend(), [&date](const NotificationInfo &info) {
                return info.dedupeKey.contains("|0|" + date + "|") &&
                       info.dedupeKey.contains("HallStaff:1");
            });
        QVERIFY(hallShortage != first.cend());
        const QString firstKey = hallShortage->dedupeKey;

        QVERIFY(execute("DELETE FROM SHIFT"));
        QVERIFY(execute(QString(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) VALUES "
            "(1002, '%1', '07:00', '12:00', 1), "
            "(1002, '%1', '07:00', '12:00', 1)").arg(date)));
        schedule.publishStaffingWarningNotifications(warningWeek);

        const QList<NotificationInfo> second = notifications.getNotifications(2001);
        auto cashierShortage = std::find_if(
            second.cbegin(), second.cend(), [&date](const NotificationInfo &info) {
                return info.dedupeKey.contains("|0|" + date + "|") &&
                       info.dedupeKey.contains("Cashier:1");
            });
        QVERIFY(cashierShortage != second.cend());
        QVERIFY(cashierShortage->dedupeKey != firstKey);
        QVERIFY(cashierShortage->message.contains(QString::fromUtf8("Thu ngân 1")));
    }

    void staffingWarningsUseDayBeforeAndNextShiftTiming()
    {
        Config::setRoles({{"Cashier", {4, 6}}});
        const QDate testDate(2026, 8, 5);
        const QDate tomorrow = testDate.addDays(1);
        QVERIFY(execute(QString(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) "
            "VALUES (1001, '%1', '07:00', '12:00', 1)")
            .arg(tomorrow.toString(Qt::ISODate))));

        Schedule_Model schedule;
        schedule.publishScheduledStaffingWarningNotifications(testDate, QTime(8, 0));
        Notification_Model notifications;
        const QList<NotificationInfo> dayBefore = notifications.getNotifications(2001);
        QVERIFY(std::any_of(dayBefore.cbegin(), dayBefore.cend(),
                            [](const NotificationInfo &notification) {
                                return notification.dedupeKey.contains("|DAY_BEFORE|");
                            }));
        const int dayBeforeCount = dayBefore.size();

        schedule.publishScheduledStaffingWarningNotifications(testDate, QTime(9, 15));
        const QList<NotificationInfo> withinThreeHours =
            notifications.getNotifications(2001);
        QVERIFY(withinThreeHours.size() > dayBeforeCount);
        QVERIFY(std::any_of(withinThreeHours.cbegin(), withinThreeHours.cend(),
                            [](const NotificationInfo &notification) {
                                return notification.dedupeKey.contains("|SHIFT_3H|");
                            }));
        QCOMPARE(withinThreeHours.first().priority, 200);
    }

    void leaveApprovalCancelsAffectedShiftAndNotifiesStaff()
    {
        const QDate leaveDate = QDate::currentDate().addDays(5);
        QSqlQuery shift(database());
        shift.prepare(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) "
            "VALUES (1002, :date, '07:00', '12:00', 1)");
        shift.bindValue(":date", leaveDate);
        QVERIFY(shift.exec());
        const int shiftId = shift.lastInsertId().toInt();

        LeaveRequest_Model leaveRequests;
        QString error;
        const QList<LeaveShiftOption> availableShifts =
            leaveRequests.getActiveShiftsForWeek(1002, leaveDate.addDays(-2));
        QCOMPARE(availableShifts.size(), 1);
        QCOMPARE(availableShifts.first().shiftId, shiftId);
        QVERIFY2(leaveRequests.submitLeaveRequest(1002, shiftId, "Medical appointment", &error),
                 qPrintable(error));

        Notification_Model notifications;
        const QList<NotificationInfo> managerNotifications =
            notifications.getNotifications(2001);
        QCOMPARE(managerNotifications.size(), 1);
        QCOMPARE(managerNotifications.first().type, QString("LEAVE_SUBMITTED"));

        const int requestId = managerNotifications.first().relatedLeaveRequestId;
        QVERIFY(requestId > 0);
        QCOMPARE(leaveRequests.getPendingLeaveRequests().size(), 1);
        QVERIFY(notifications.markAllAsRead(2001));
        QCOMPARE(notifications.getNotifications(2001).first().status, QString("Read"));
        QVERIFY2(leaveRequests.decideLeaveRequest(requestId, 2001, true,
                                                   "Approved", &error),
                 qPrintable(error));
        QVERIFY(!leaveRequests.decideLeaveRequest(requestId, 2001, true,
                                                   "Duplicate", &error));
        QVERIFY(notifications.markLeaveRequestReviewedByRequest(requestId, true));
        const QList<NotificationInfo> reviewedManagerNotifications =
            notifications.getNotifications(2001);
        QCOMPARE(reviewedManagerNotifications.first().type, QString("LEAVE_APPROVED"));
        QCOMPARE(reviewedManagerNotifications.first().status, QString("Read"));
        QCOMPARE(leaveRequests.getPendingLeaveRequests().size(), 0);
        QCOMPARE(scalar("SELECT status FROM SHIFT WHERE idEmployee = 1002"), -2);
        QCOMPARE(scalar("SELECT COUNT(*) FROM SHIFT_AUDIT WHERE action = 'cancel'"), 1);

        const QList<NotificationInfo> staffNotifications =
            notifications.getNotifications(1002);
        QCOMPARE(staffNotifications.size(), 1);
        QCOMPARE(staffNotifications.first().type, QString("LEAVE_APPROVED"));
    }

    void nonManagerCannotDecideLeaveRequest()
    {
        const QDate leaveDate = QDate::currentDate().addDays(7);
        QSqlQuery shift(database());
        shift.prepare(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) "
            "VALUES (1002, :date, '07:00', '12:00', 0)");
        shift.bindValue(":date", leaveDate);
        QVERIFY(shift.exec());

        LeaveRequest_Model leaveRequests;
        QString error;
        QVERIFY(leaveRequests.submitLeaveRequest(1002, shift.lastInsertId().toInt(),
                                                  "Personal matter", &error));
        Notification_Model notifications;
        const int requestId = notifications.getNotifications(2001).first()
                                  .relatedLeaveRequestId;
        QVERIFY(!leaveRequests.decideLeaveRequest(requestId, 1001, true,
                                                   "Unauthorized", &error));
        QCOMPARE(scalar("SELECT status FROM LEAVE_REQUEST"), 0);
    }

    void leaveDeclineNotifiesStaffWithoutChangingShift()
    {
        const QDate leaveDate = QDate::currentDate().addDays(6);
        QSqlQuery shift(database());
        shift.prepare(
            "INSERT INTO SHIFT (idEmployee, workDate, startTime, endTime, status) "
            "VALUES (1002, :date, '12:00', '17:00', 0)");
        shift.bindValue(":date", leaveDate);
        QVERIFY(shift.exec());
        LeaveRequest_Model leaveRequests;
        QString error;
        QVERIFY2(leaveRequests.submitLeaveRequest(1002, shift.lastInsertId().toInt(),
                                                   "Personal matter", &error),
                 qPrintable(error));
        Notification_Model notifications;
        const int requestId = notifications.getNotifications(2001).first()
                                  .relatedLeaveRequestId;
        QVERIFY2(leaveRequests.decideLeaveRequest(requestId, 2001, false,
                                                   "Insufficient coverage", &error),
                 qPrintable(error));
        const QList<NotificationInfo> staffNotifications =
            notifications.getNotifications(1002);
        QCOMPARE(staffNotifications.size(), 1);
        QCOMPARE(staffNotifications.first().type, QString("LEAVE_DECLINED"));
        QCOMPARE(scalar("SELECT status FROM SHIFT"), 0);
    }
};

QTEST_MAIN(SchedulePersistenceTests)
#include "SchedulePersistenceTests.moc"
