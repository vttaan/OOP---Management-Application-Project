#pragma once

#include "global.h"
#include "utils/Database.h"

enum class PasswordChangeResult {
    SUCCESS,
    WRONG_OLD_PASSWORD,
    NEW_PASSWORD_TOO_WEAK,
    DATABASE_ERROR
};

class Change_password {
public:
    PasswordChangeResult updatePassword(short int employeeId, 
                                        const QString& oldPassword, 
                                        const QString& newPassword);

private:
    bool verifyOldPassword(short int employeeId, const QString& oldPassword);
    bool validatePasswordStrength(const QString& newPassword);
    bool executePasswordUpdate(short int employeeId, const QString& newHashedPassword);
};