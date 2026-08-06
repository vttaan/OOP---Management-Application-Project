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

    queryAccount.prepare("SELECT passWord FROM ACCOUNTS WHERE idEmployee = :id");
    queryAccount.bindValue(":id", employeeId);

    if (!queryAccount.exec() || !queryAccount.next()) {
        return false;
    }

    const QString storedPassword = queryAccount.value("passWord").toString();
    return Security::unwrapPasswordHash(storedPassword) == Security::hashPassword(oldPassword);
}

QString Change_password::getPasswordStrengthError(const QString& newPassword)
{
    if (newPassword.isEmpty()) {
        return "Mật khẩu không được để trống.";
    }
    if (newPassword.length() < 6) {
        return "Mật khẩu phải có ít nhất 6 ký tự.";
    }
    if (newPassword.contains('$')) {
        return "Mật khẩu không được chứa ký tự '$'.";
    }

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;

    for (const QChar& ch : newPassword) {
        if (ch.isUpper()) {
            hasUpper = true;
        } else if (ch.isLower()) {
            hasLower = true;
        } else if (ch.isDigit()) {
            hasDigit = true;
        } else if (!ch.isSpace()) {
            hasSpecial = true;
        }
    }

    if (!hasUpper) {
        return "Mật khẩu cần ít nhất 1 chữ hoa (A-Z).";
    }
    if (!hasLower) {
        return "Mật khẩu cần ít nhất 1 chữ thường (a-z).";
    }
    if (!hasDigit) {
        return "Mật khẩu cần ít nhất 1 chữ số (0-9).";
    }
    if (!hasSpecial) {
        return "Mật khẩu cần ít nhất 1 ký tự đặc biệt.";
    }

    return QString();
}

bool Change_password::validatePasswordStrength(const QString& newPassword)
{
    return getPasswordStrengthError(newPassword).isEmpty();
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
