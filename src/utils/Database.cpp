#include "utils/Database.h"
#include <QDebug>

Database* Database::instance = nullptr;

Database::Database() {
    this->dbConnect = QSqlDatabase::addDatabase("QSQLITE");
    qDebug() << "FOLDER CONTAINS DATABASE\n" << QDir::currentPath() << '\n';
    dbConnect.setDatabaseName("database/Systems.db");

    // isOpen -> open
    if (!dbConnect.open()) qDebug() << "ERROR CAN NOT OPEN DATABASE\n";

	else qDebug() << "OPEN DATABASE SUCCESS\n";
}

Database* Database::getInstance() {
    return Database::instance != nullptr ? Database::instance : Database::instance = new Database();
    //return Database::instance != nullptr ? Database::instance : new Database();
}

QSqlDatabase Database::getDbConnect() {
	return this->dbConnect;
}

void Database::closeConnect() {
	if (this->dbConnect.isOpen()) {
		this->dbConnect.close();
		qDebug() << "CLOSE DATABASE SUCCESS\n";
	}
}

QSqlQuery Database::execQuery(const QString& query) {
	QSqlQuery ansForQuery(this->dbConnect);
    //if (ansForQuery.exec(query))
    if (!ansForQuery.exec(query)) {
		ansForQuery.exec(query);
		qDebug() << "ERROR EXEC QUERY " << ansForQuery.lastError().text() << '\n';
	}
	return ansForQuery;
}

