#include <QtTest>
#include <memory>

#include "model/Login_Model.h"
#include "model/Salary_Model.h"
#include "model/Schedule_Model.h"
#include "model/Notification_Model.h"
#include "model/LeaveRequest_Model.h"
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
            "(2001, 'Manager', 0)"));
        QSqlQuery account(database());
        account.prepare(
            "INSERT INTO ACCOUNTS (idEmployee, userName, passWord) "
            "VALUES (1001, 'fixed.user', :password)");
        account.bindValue(":password", Security::hashPassword("secret"));
        QVERIFY(account.exec());
    }

    void init()
    {
        QVERIFY(execute("DROP TRIGGER IF EXISTS reject_evening"));
        QVERIFY(execute("DELETE FROM SHIFT"));
        QVERIFY(execute("DELETE FROM NOTIFICATION"));
        QVERIFY(execute("DELETE FROM LEAVE_REQUEST"));
        QVERIFY(execute("DELETE FROM SHIFT_AUDIT"));
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
