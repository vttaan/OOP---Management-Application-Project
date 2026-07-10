#pragma once
#include <QSqlDatabase>
#include <QString>
#include <QSqlQuery>
#include <QSqlError>
#include <QCoreApplication>
#include <QDir>
class Database {
private:
	static Database* instance;
	QSqlDatabase dbConnect;
	Database();
public:
	static Database* getInstance();
	QSqlQuery execQuery(const QString& query);
	void closeConnect();
	QSqlDatabase getDbConnect();
};