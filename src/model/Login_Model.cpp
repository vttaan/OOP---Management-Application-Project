#include "model/Login_Model.h"
#include "utils/Database.h"
#include "core/UserFactory.h"
#include "utils/Security.h"
User* Login_Model::verifyLogin(const QString& userName, const QString& password) {
	QSqlDatabase openData = Database::getInstance()->getDbConnect();
	QSqlQuery queryAccount(openData);


	// CONSIDER HASH IN CONTROL
	QString hashedPassWord = Security::hashPassword(password);
	
	queryAccount.prepare("SECLECT * FROM ACCOUNTS WHERE userName = :u AND passWord = :v");
	queryAccount.bindValue(":u", userName);
	queryAccount.bindValue(":v", hashedPassWord);
	if (!queryAccount.exec() || !queryAccount.next()) return nullptr;

	short int idEmployee = queryAccount.value("idEmployee").toInt();

	QSqlQuery queryProfile(openData);
	queryProfile.prepare("SECLECT * FROM PROFILES WHERE idEmployee = :u");
	queryProfile.bindValue(":u", idEmployee);

	if (!queryProfile.exec() || !queryProfile.next()) return nullptr;

	QString curRole = queryProfile.value("role").toString();
	QString curIdIndentity = queryProfile.value("IdCitizenIndentity").toString();
	QString curName = queryProfile.value("name").toString();
	QString curPhone = queryProfile.value("phoneNum").toString();
	QString curDob = queryProfile.value("dob").toString();
	QString curAddress = queryProfile.value("address").toString();
	QString curAvatarPath = queryProfile.value("avatarPath").toString();

	User* curUser = UserFactory::createNewUser(curRole, idEmployee, curAvatarPath, curIdIndentity, curName, curDob, curAddress, 
		curPhone);

	return curUser;
}