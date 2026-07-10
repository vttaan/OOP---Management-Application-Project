#pragma once

#include "global.h"
#include "model/Change_password.h"  // Provides PasswordChangeResult enum + Change_password class

class Profile_Model {
public:
    bool updateProfile(short int idEmployee, const QString& name, const QString& dob,
                       const QString& address, const QString& phoneNum,
                       const QString& citizenId, const QString& avatarPath);

    // Delegates to Change_password for verification, hashing, and DB update.
    // Returns a PasswordChangeResult so the controller can show precise error messages.
    PasswordChangeResult updatePassword(short int idEmployee,
                                        const QString& oldPassword,
                                        const QString& newPassword);

    // Checks whether the supplied plain-text password matches the stored hash.
    bool checkIfUserExist(short int idEmployee, const QString& password);
};
