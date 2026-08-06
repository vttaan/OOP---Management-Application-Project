#include "global.h"
#include "model/Login_Model.h"
#include "utils/Database.h"
#include "core/UserFactory.h"
#include "utils/Security.h"
User *Login_Model::verifyLogin(const QString &userName, const QString &password,
                               bool *mustChangeInitialPassword)
{
    if (mustChangeInitialPassword)
        *mustChangeInitialPassword = false;

    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery queryAccount(openData);
    QString hashedPassWord = Security::hashPassword(password);

    // Fetch the stored value first because a first-login account has an INIT$
    // marker outside its hash. Existing accounts remain stored as a plain hash.
    queryAccount.prepare("SELECT * FROM ACCOUNTS WHERE userName = :u");
    queryAccount.bindValue(":u", userName);

    if (!queryAccount.exec() || !queryAccount.next()) return nullptr;

    const QString storedPassword = queryAccount.value("passWord").toString();
    const bool isInitialPassword = Security::isInitialPasswordHash(storedPassword);
    if (Security::unwrapPasswordHash(storedPassword) != hashedPassWord)
        return nullptr;

    if (mustChangeInitialPassword)
        *mustChangeInitialPassword = isInitialPassword;

    short int idEmployee = queryAccount.value("idEmployee").toInt();

QSqlQuery queryProfile(openData);
queryProfile.prepare("SELECT * FROM PROFILES WHERE idEmployee = :u");
queryProfile.bindValue(":u", idEmployee);
if(!queryProfile.exec() || !queryProfile.next()) {
    qDebug() << "ERROR NOT EXEC QUERY IN VERIFY LOGIN\n";
    return nullptr;
} else {
    qDebug() << "EXEC QUERY VERIFY LOGIN SUCCESS\n";
}
QString curRole = queryProfile.value("role").toString();
QString curIdIdentity = queryProfile.value("IdCitizenIdentity").toString();
QString curName = queryProfile.value("name").toString();
QString curPhone = queryProfile.value("phoneNum").toString();
QString curDob = queryProfile.value("dob").toString();
QString curAddress = queryProfile.value("address").toString();
QString curAvatarPath = queryProfile.value("avatarPath").toString();
QString curGender = queryProfile.value("Gender").toString();
int curSalary = queryProfile.value("Salary").toInt();
bool curIsFixed = queryProfile.value("isFixed").toBool();

User *curUser = UserFactory::createContainsUser(curRole, idEmployee, curAvatarPath, curIdIdentity, curName, curDob, curAddress,
                                                curPhone, curGender, curSalary, curIsFixed);
return curUser;
}
