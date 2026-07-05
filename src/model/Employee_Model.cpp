#include "model/Employee_Model.h"
#include "utils/Database.h"
#include "utils/Security.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>

Employee_Model::Employee_Model() {}

QList<User *> Employee_Model::getAllEmployees()
{
    QList<User *> list;

    QSqlDatabase db = Database::getInstance()->getDbConnect();
    QSqlQuery query("SELECT * FROM PROFILES", db); // Fixed: QSqlQuery (was QSqlDatabase)

    while (query.next())
    {
        short int id = query.value("idEmployee").toInt();
        QString role = query.value("role").toString();
        QString name = query.value("name").toString();
        QString phone = query.value("phoneNum").toString();
        QString dob = query.value("dob").toString();
        QString address = query.value("address").toString();
        QString avatar = query.value("avatarPath").toString();
        QString citizenId = query.value("IdCitizenIndentity").toString();

        User *emp = UserFactory::createContainsUser(role, id, avatar, citizenId, name, dob, address, phone); // Fixed: citizenID → citizenId
        if (emp)
        {
            list.append(emp);
        }
    }
    return list;
}

bool Employee_Model::addEmployee(User *emp, const QString &username, const QString &password)
{
    QSqlDatabase db = Database::getInstance()->getDbConnect();
    db.transaction(); // start transaction before inserting

    // Copy avatar to local folder and update user object
    QString localAva = saveAvatarLocally(emp->getIdEmployee(), emp->getAvatarPath());
    emp->setAva(localAva);

    QSqlQuery qProfile(db);
    qProfile.prepare("INSERT INTO PROFILES (idEmployee, role, name, phoneNum, dob, address, avatarPath, IdCitizenIndentity) "
                     "VALUES (:id,:role,:name,:phone,:dob,:address,:avatar,:citizen)");
    qProfile.bindValue(":id", emp->getIdEmployee());
    qProfile.bindValue(":role", emp->getRole());
    qProfile.bindValue(":name", emp->getName());
    qProfile.bindValue(":phone", emp->getPhoneNum());
    qProfile.bindValue(":dob", emp->getDOB());
    qProfile.bindValue(":address", emp->getAddress());
    qProfile.bindValue(":avatar", emp->getAvatarPath());
    qProfile.bindValue(":citizen", emp->getIndentityID());

    if (!qProfile.exec())
    {
        qDebug() << "Error adding profile" << qProfile.lastError().text();
        db.rollback();
        return false;
    }

    if (!username.isEmpty() && !password.isEmpty())
    {
        QSqlQuery qAccount(db);
        QString hashedPass = Security::hashPassword(password);
        qAccount.prepare("INSERT INTO ACCOUNTS (userName, passWord, idEmployee) VALUES (:user, :pass, :id)");
        qAccount.bindValue(":user", username);
        qAccount.bindValue(":pass", hashedPass);
        qAccount.bindValue(":id", emp->getIdEmployee());

        if (!qAccount.exec())
        {
            qDebug() << "Error adding account:" << qAccount.lastError().text();
            db.rollback();
            return false;
        }
    }

    return db.commit();
}

bool Employee_Model::updateEmployee(User *emp)
{
    if (!emp)
        return false;

    // Copy avatar to local folder and update user object
    QString localAva = saveAvatarLocally(emp->getIdEmployee(), emp->getAvatarPath());
    emp->setAva(localAva);

    QSqlDatabase db = Database::getInstance()->getDbConnect();
    QSqlQuery query(db);
    query.prepare("UPDATE PROFILES SET role = :role, name = :name, phoneNum = :phone, dob = :dob, "
                  "address = :address, avatarPath = :avatar, IdCitizenIndentity = :citizen WHERE idEmployee = :id");
    query.bindValue(":role", emp->getRole());
    query.bindValue(":name", emp->getName());
    query.bindValue(":phone", emp->getPhoneNum());
    query.bindValue(":dob", emp->getDOB());
    query.bindValue(":address", emp->getAddress());
    query.bindValue(":avatar", emp->getAvatarPath());
    query.bindValue(":citizen", emp->getIndentityID());
    query.bindValue(":id", emp->getIdEmployee());

    if (!query.exec())
    {
        qDebug() << "Error updating profile:" << query.lastError().text();
        return false;
    }
    return true;
}

bool Employee_Model::deleteEmployee(int idEmployee)
{
    QSqlDatabase db = Database::getInstance()->getDbConnect();
    db.transaction();

    QSqlQuery qAccount(db);
    qAccount.prepare("DELETE FROM ACCOUNTS WHERE idEmployee = :id");
    qAccount.bindValue(":id", idEmployee);
    qAccount.exec();

    QSqlQuery qProfile(db);
    qProfile.prepare("DELETE FROM PROFILES WHERE idEmployee = :id");
    qProfile.bindValue(":id", idEmployee);

    if (!qProfile.exec())
    {
        qDebug() << "Error deleting profile:" << qProfile.lastError().text();
        db.rollback();
        return false;
    }

    return db.commit();
}

QString Employee_Model::saveAvatarLocally(int empId, const QString &sourcePath)
{
    if (sourcePath.isEmpty())
        return "";

    // If it's already a resource path, do not copy
    if (sourcePath.startsWith(":/"))
        return sourcePath;

    QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists())
        return "";

    // Create "avatars" directory in application directory if not exists
    QString targetDir = QCoreApplication::applicationDirPath() + "/avatars";
    QDir dir(targetDir);
    if (!dir.exists())
    {
        dir.mkpath(".");
    }

    // Generate path: appDir/avatars/avatar_X.ext
    QString ext = sourceInfo.suffix().toLower();
    if (ext.isEmpty())
        ext = "png";
    QString targetPath = QString("%1/avatar_%2.%3").arg(targetDir).arg(empId).arg(ext);

    // If file already exists, remove it first to overwrite
    if (QFile::exists(targetPath))
    {
        QFile::remove(targetPath);
    }

    if (QFile::copy(sourcePath, targetPath))
    {
        return targetPath;
    }

    return sourcePath;
}
