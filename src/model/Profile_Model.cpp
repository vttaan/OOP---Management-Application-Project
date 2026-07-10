#include "global.h"
#include "model/Profile_Model.h"
#include "utils/Database.h"
#include "utils/Security.h"

bool Profile_Model::updateProfile(short int idEmployee, const QString& name, const QString& dob, const QString& address, const QString& phoneNum, const QString& citizenId, const QString& avatarPath) {
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);
    
    query.prepare("UPDATE PROFILES SET name = :name, dob = :dob, address = :address, phoneNum = :phoneNum, IdCitizenIdentity = :citizenId, avatarPath = :avatarPath WHERE idEmployee = :id");
    query.bindValue(":name", name);
    query.bindValue(":dob", dob);
    query.bindValue(":address", address);
    query.bindValue(":phoneNum", phoneNum);
    query.bindValue(":citizenId", citizenId);
    query.bindValue(":avatarPath", avatarPath);
    query.bindValue(":id", idEmployee);
    
    if (!query.exec()) {
        qDebug() << "ERROR UPDATING PROFILE:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "UPDATE PROFILE SUCCESSFUL";
    return true;
}

// Delegates fully to Change_password which handles:
//   1. New password strength validation
//   2. Old password verification (hashed lookup in ACCOUNTS)
//   3. Hashing the new password
//   4. Updating ACCOUNTS.passWord
PasswordChangeResult Profile_Model::updatePassword(short int idEmployee,
                                                    const QString& oldPassword,
                                                    const QString& newPassword)
{
    Change_password cp;
    return cp.updatePassword(idEmployee, oldPassword, newPassword);
}

// Checks that the employee account exists and the provided password matches the stored hash.
bool Profile_Model::checkIfUserExist(short int idEmployee, const QString& password) {
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);

    QString hashedPassword = Security::hashPassword(password);

    query.prepare("SELECT passWord FROM ACCOUNTS WHERE idEmployee = :id");
    query.bindValue(":id", idEmployee);

    if (!query.exec()) {
        qDebug() << "NO ACCOUNT FOUND:" << query.lastError().text();
        return false;
    }

    if (!query.next()) {
        qDebug() << "ACCOUNT DOES NOT EXIST for id" << idEmployee;
        return false;
    }

    if (query.value("passWord").toString() != hashedPassword) {
        qDebug() << "WRONG PASSWORD";
        return false;
    }

    qDebug() << "ACCOUNT VERIFIED for id" << idEmployee;
    return true;
}