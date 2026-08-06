#pragma once

#include "global.h"
class Security {
public:
	static QString hashPassword(const QString& password);
	static QString markInitialPasswordHash(const QString& passwordHash);
	static bool isInitialPasswordHash(const QString& storedPassword);
	static QString unwrapPasswordHash(const QString& storedPassword);
};
