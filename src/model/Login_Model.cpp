#include "model/Login_Model.h"
#include "utils/Database.h"
#include "core/UserFactory.h"
#include "utils/Security.h"
User *Login_Model::verifyLogin(const QString &userName, const QString &password)
{
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery queryAccount(openData);
    queryAccount.prepare("SELECT * FROM ACCOUNTS");

    // CONSIDER HASH IN CONTROL
    QString hashedPassWord = Security::hashPassword(password);

    queryAccount.prepare("SELECT * FROM ACCOUNTS WHERE userName = :u AND passWord = :v");
    queryAccount.bindValue(":u", userName);
    queryAccount.bindValue(":v", hashedPassWord);

    if (!queryAccount.exec() || !queryAccount.next()) return nullptr;

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

User *curUser = UserFactory::createContainsUser(curRole, idEmployee, curAvatarPath, curIdIdentity, curName, curDob, curAddress,
                                                curPhone, curGender);
return curUser;
>>>>>>> Tin
}