#pragma once

#include "global.h"
class Database {
private:
	static Database* instance;
	QSqlDatabase dbConnect;
	Database();
	void ensureSchema();
public:
	static Database* getInstance();
	QSqlQuery execQuery(const QString& query);
	void closeConnect();
	QSqlDatabase getDbConnect();
};
