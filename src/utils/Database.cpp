#include "utils/Database.h"
#include <QDebug>

Database* Database::instance = nullptr;

Database::Database() {
	this->dbConnect = QSqlDatabase::addDatabase("QSQLite");
	dbConnect.setDatabaseName("database/Systems.db");
	
	if (!dbConnect.isOpen()) qDebug() << "ERROR CAN NOT OPEN DATABASE\n";
	else qDebug() << "OPEN DATABASE SUCCESS\n";
}

Database* Database::getInstance() {
	return Database::instance != nullptr ? Database::instance : new Database();
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
	if (ansForQuery.exec(query)) {
		ansForQuery.exec(query);
		qDebug() << "ERROR EXEC QUERY " << ansForQuery.lastError().text() << '\n';
	}
	return ansForQuery;
}

