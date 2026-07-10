#pragma once

#include "global.h"
class Profile_Model {
public:
    bool updateProfile(short int idEmployee, const QString& name, const QString& dob, const QString& address, const QString& phoneNum, const QString& citizenId, const QString& avatarPath);
    bool updatePassword(short int idEmployee, const QString& password);
    bool checkIfUserExist(short int idEmployee, const QString& password);
};
