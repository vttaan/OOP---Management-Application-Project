#pragma once

#include "global.h"
class Security {
public:
	static QString hashPassword(const QString& password);
};