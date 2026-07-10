#pragma once

#include "global.h"
#include "core/User.h"
class Login_Model {
public:
	User* verifyLogin(const QString& userName, const QString& password);
	void logOut();

};