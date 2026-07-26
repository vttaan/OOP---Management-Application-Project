#include "global.h"
#include "model/Change_password.h"
#include "utils/Security.h"

PasswordChangeResult Change_password::updatePassword(short int employeeId, 
                                                     const QString& oldPassword, 
                                                     const QString& newPassword)
{
    // Step 1: Validate new password strength
    if (!validatePasswordStrength(newPassword)) {
        return PasswordChangeResult::NEW_PASSWORD_TOO_WEAK;
    }

    // Step 2: Verify old password
    if (!verifyOldPassword(employeeId, oldPassword)) {
        return PasswordChangeResult::WRONG_OLD_PASSWORD;
    }

    // Step 3: Hash new password
    QString newHashedPassword = Security::hashPassword(newPassword);

    // Step 4: Update database
    if (!executePasswordUpdate(employeeId, newHashedPassword)) {
        return PasswordChangeResult::DATABASE_ERROR;
    }

    return PasswordChangeResult::SUCCESS;
}

bool Change_password::verifyOldPassword(short int employeeId, const QString& oldPassword)
{
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery queryAccount(openData);
    
    QString hashedOldPassword = Security::hashPassword(oldPassword);

    queryAccount.prepare("SELECT * FROM ACCOUNTS WHERE idEmployee = :id AND passWord = :pw");
    queryAccount.bindValue(":id", employeeId);
    queryAccount.bindValue(":pw", hashedOldPassword);

    if (!queryAccount.exec() || !queryAccount.next()) {
        return false;
    }

    return true;
}

bool Change_password::validatePasswordStrength(const QString& newPassword)
{
    // Basic validation: at least 6 characters
    if (newPassword.length() < 6) {
        return false;
    }
    return true;
}

bool Change_password::executePasswordUpdate(short int employeeId, const QString& newHashedPassword)
{
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery queryAccount(openData);

    queryAccount.prepare("UPDATE ACCOUNTS SET passWord = :pw WHERE idEmployee = :id");
    queryAccount.bindValue(":pw", newHashedPassword);
    queryAccount.bindValue(":id", employeeId);

    if (!queryAccount.exec()) {
        qDebug() << "Failed to update password for employee ID:" << employeeId;
        return false;
    }

    return true;
}
