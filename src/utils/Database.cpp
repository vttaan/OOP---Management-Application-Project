#include "global.h"
#include "utils/Database.h"

Database *Database::instance = nullptr;

Database::Database()
{
    this->dbConnect = QSqlDatabase::addDatabase("QSQLITE");
    qDebug() << "FOLDER CONTAINS DATABASE\n"
             << QDir::currentPath() << '\n';
    dbConnect.setDatabaseName("database/Systems.db");

    // isOpen -> open
    if (!dbConnect.open())
    {
        qDebug() << "ERROR CAN NOT OPEN DATABASE\n";
    }
    else
    {
        qDebug() << "OPEN DATABASE SUCCESS\n";
        QSqlQuery alterQuery(dbConnect);
        alterQuery.exec("ALTER TABLE PROFILES ADD COLUMN status TEXT DEFAULT 'active'");
        ensureSchema();
    }
}

void Database::ensureSchema()
{
    QSqlQuery query(dbConnect);
    const QStringList statements = {
        "CREATE TABLE IF NOT EXISTS SHIFT_AUDIT ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, shiftId INTEGER, "
        "employeeId INTEGER NOT NULL, action TEXT NOT NULL, reason TEXT, "
        "changedAt TEXT NOT NULL)",
        "CREATE TABLE IF NOT EXISTS LEAVE_REQUEST ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, idEmployee INTEGER NOT NULL, "
        "leaveDate TEXT NOT NULL, relatedShiftId INTEGER, reason TEXT NOT NULL, "
        "status TEXT NOT NULL DEFAULT 'Pending', requestedAt TEXT NOT NULL, "
        "decidedAt TEXT, decidedBy INTEGER, decisionReason TEXT)",
        "CREATE TABLE IF NOT EXISTS NOTIFICATION ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, recipientEmployeeId INTEGER NOT NULL, "
        "type TEXT NOT NULL, title TEXT NOT NULL, message TEXT NOT NULL, "
        "status TEXT NOT NULL DEFAULT 'Unread', relatedShiftId INTEGER, "
        "relatedLeaveRequestId INTEGER, createdAt TEXT NOT NULL, readAt TEXT)",
        "CREATE INDEX IF NOT EXISTS idx_notification_recipient "
        "ON NOTIFICATION(recipientEmployeeId, status, createdAt)",
        "CREATE INDEX IF NOT EXISTS idx_leave_request_employee "
        "ON LEAVE_REQUEST(idEmployee, status, leaveDate)"};

    for (const QString &statement : statements)
    {
        if (!query.exec(statement))
            qWarning() << "Schema migration failed:" << query.lastError().text();
    }

    bool hasRelatedShiftColumn = false;
    if (query.exec("PRAGMA table_info(LEAVE_REQUEST)"))
    {
        while (query.next())
        {
            if (query.value(1).toString() == "relatedShiftId")
            {
                hasRelatedShiftColumn = true;
                break;
            }
        }
    }
    if (!hasRelatedShiftColumn &&
        !query.exec("ALTER TABLE LEAVE_REQUEST ADD COLUMN relatedShiftId INTEGER"))
    {
        qWarning() << "Leave request migration failed:" << query.lastError().text();
    }
}

Database *Database::getInstance()
{
    return Database::instance != nullptr ? Database::instance : Database::instance = new Database();
    // return Database::instance != nullptr ? Database::instance : new Database();
}

QSqlDatabase Database::getDbConnect()
{
    return this->dbConnect;
}

void Database::closeConnect()
{
    if (this->dbConnect.isOpen())
    {
        this->dbConnect.close();
        qDebug() << "CLOSE DATABASE SUCCESS\n";
    }
}

QSqlQuery Database::execQuery(const QString &query)
{
    QSqlQuery ansForQuery(this->dbConnect);
    // if (ansForQuery.exec(query))
    if (!ansForQuery.exec(query))
    {
        ansForQuery.exec(query);
        qDebug() << "ERROR EXEC QUERY " << ansForQuery.lastError().text() << '\n';
    }
    return ansForQuery;
}
