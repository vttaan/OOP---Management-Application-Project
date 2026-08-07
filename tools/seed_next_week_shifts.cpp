#include <QCoreApplication>
#include <QDate>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextStream>
#include <QVariant>

namespace
{
struct ShiftBlock
{
    const char *start;
    const char *end;
};

const ShiftBlock blocks[] = {
    {"07:00:00.000", "12:00:00.000"},
    {"12:00:00.000", "17:00:00.000"},
    {"17:00:00.000", "22:00:00.000"},
};

void printUsage(QTextStream &out)
{
    out << "Usage: seed_next_week_shifts [--database PATH] [--week YYYY-MM-DD]"
           " [--replace-pending]\n"
        << "\n"
        << "Seeds one non-overlapping pending shift per active employee per day.\n"
        << "The default week is next Monday through Sunday.\n"
        << "Existing approved shifts are never changed.\n"
        << "--replace-pending deletes pending rows in the target week before seeding.\n";
}

bool hasTable(QSqlDatabase &db, const QString &table)
{
    QSqlQuery query(db);
    query.prepare("SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = :table");
    query.bindValue(":table", table);
    return query.exec() && query.next();
}

QDate mondayFor(QDate date)
{
    return date.addDays(1 - date.dayOfWeek());
}
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);
    QTextStream err(stderr);

    QString databasePath = "database/Systems.db";
    QDate weekStart = mondayFor(QDate::currentDate()).addDays(7);
    bool replacePending = false;

    const QStringList args = app.arguments();
    for (int i = 1; i < args.size(); ++i)
    {
        const QString &arg = args.at(i);
        if (arg == "--help" || arg == "-h")
        {
            printUsage(out);
            return 0;
        }
        if (arg == "--replace-pending")
        {
            replacePending = true;
            continue;
        }
        if (arg == "--database" && i + 1 < args.size())
        {
            databasePath = args.at(++i);
            continue;
        }
        if (arg == "--week" && i + 1 < args.size())
        {
            const QDate requested = QDate::fromString(args.at(++i), Qt::ISODate);
            if (!requested.isValid())
            {
                err << "Invalid --week date. Use YYYY-MM-DD.\n";
                return 2;
            }
            weekStart = mondayFor(requested);
            continue;
        }

        err << "Unknown or incomplete argument: " << arg << "\n";
        printUsage(err);
        return 2;
    }

    const QString connectionName = "seed_next_week_shifts_connection";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(databasePath);
    if (!db.open())
    {
        err << "Cannot open database " << databasePath << ": "
            << db.lastError().text() << "\n";
        return 1;
    }

    if (!hasTable(db, "PROFILES") || !hasTable(db, "SHIFT"))
    {
        err << "The database must contain PROFILES and SHIFT tables.\n";
        return 1;
    }

    QSqlQuery employees(db);
    if (!employees.exec("SELECT idEmployee FROM PROFILES "
                        "WHERE status IS NULL OR lower(status) = 'active' "
                        "ORDER BY idEmployee"))
    {
        err << "Cannot read active employees: " << employees.lastError().text() << "\n";
        return 1;
    }

    QList<int> employeeIds;
    while (employees.next())
        employeeIds.append(employees.value(0).toInt());

    if (employeeIds.isEmpty())
    {
        err << "No active employees were found; no rows were created.\n";
        return 1;
    }

    if (!db.transaction())
    {
        err << "Cannot start transaction: " << db.lastError().text() << "\n";
        return 1;
    }

    const QDate weekEnd = weekStart.addDays(6);
    if (replacePending)
    {
        QSqlQuery remove(db);
        remove.prepare("DELETE FROM SHIFT WHERE status = 0 "
                       "AND workDate BETWEEN :start AND :end");
        remove.bindValue(":start", weekStart.toString(Qt::ISODate));
        remove.bindValue(":end", weekEnd.toString(Qt::ISODate));
        if (!remove.exec())
        {
            err << "Cannot replace pending rows: " << remove.lastError().text() << "\n";
            db.rollback();
            return 1;
        }
    }

    QSqlQuery overlap(db);
    overlap.prepare("SELECT 1 FROM SHIFT WHERE idEmployee = :employee "
                    "AND workDate = :date AND status IN (0, 1) "
                    "AND startTime < :end AND endTime > :start LIMIT 1");

    QSqlQuery insert(db);
    insert.prepare("INSERT INTO SHIFT "
                   "(idEmployee, workDate, startTime, endTime, status, isHoliday) "
                   "VALUES (:employee, :date, :start, :end, 0, 0)");

    int inserted = 0;
    int skipped = 0;
    for (int day = 0; day < 7; ++day)
    {
        const QDate date = weekStart.addDays(day);
        for (int employeeIndex = 0; employeeIndex < employeeIds.size(); ++employeeIndex)
        {
            // Rotate blocks by day and employee so each employee has one shift/day
            // and the generated data exercises all three scheduling blocks.
            const ShiftBlock &block = blocks[(day + employeeIndex) % 3];
            overlap.bindValue(":employee", employeeIds.at(employeeIndex));
            overlap.bindValue(":date", date.toString(Qt::ISODate));
            overlap.bindValue(":start", block.start);
            overlap.bindValue(":end", block.end);
            if (!overlap.exec())
            {
                err << "Cannot check existing shifts: " << overlap.lastError().text() << "\n";
                db.rollback();
                return 1;
            }
            if (overlap.next())
            {
                ++skipped;
                continue;
            }

            insert.bindValue(":employee", employeeIds.at(employeeIndex));
            insert.bindValue(":date", date.toString(Qt::ISODate));
            insert.bindValue(":start", block.start);
            insert.bindValue(":end", block.end);
            if (!insert.exec())
            {
                err << "Cannot insert shift: " << insert.lastError().text() << "\n";
                db.rollback();
                return 1;
            }
            ++inserted;
        }
    }

    if (!db.commit())
    {
        err << "Cannot commit seeded shifts: " << db.lastError().text() << "\n";
        db.rollback();
        return 1;
    }

    out << "Seeded week " << weekStart.toString(Qt::ISODate) << " to "
        << weekEnd.toString(Qt::ISODate) << "\n"
        << "Active employees: " << employeeIds.size() << "\n"
        << "Pending rows inserted: " << inserted << "\n"
        << "Rows skipped because an active shift already exists: " << skipped << "\n";
    return 0;
}
