#pragma once
#include<QString>
#include <QCryptographicHash>
#include <QByteArray>

class Security {
public:
	static QString hashPassword(const QString& password);
};