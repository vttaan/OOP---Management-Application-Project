#include "utils/Security.h"


QString Security::hashPassword(const QString& password) {
	QByteArray passToHashPass = password.toUtf8();
	passToHashPass = QCryptographicHash::hash(passToHashPass, QCryptographicHash::Sha256);
	return QString(passToHashPass.toHex());
}