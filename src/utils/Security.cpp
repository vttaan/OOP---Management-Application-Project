#include "global.h"
#include "utils/Security.h"

QString Security::hashPassword(const QString& password) {
	QByteArray passToHashPass = password.toUtf8();
	passToHashPass = QCryptographicHash::hash(passToHashPass, QCryptographicHash::Sha256);
	return QString(passToHashPass.toHex());
}

QString Security::markInitialPasswordHash(const QString& passwordHash) {
    return QStringLiteral("INIT$") + passwordHash;
}

bool Security::isInitialPasswordHash(const QString& storedPassword) {
    return storedPassword.startsWith(QStringLiteral("INIT$"));
}

QString Security::unwrapPasswordHash(const QString& storedPassword) {
    return isInitialPasswordHash(storedPassword)
        ? storedPassword.mid(QStringLiteral("INIT$").size())
        : storedPassword;
}
