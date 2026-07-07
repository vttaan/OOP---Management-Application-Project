#include "model/Profile_Model.h"
#include "utils/Database.h"
#include <QDebug>

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


// AI MERGE CODE THI DOI FUNCTION NAY LAY FUNCTION PASSWORD MODEL CUA HUNG MEL BEL
bool Profile_Model::updatePassword(short int idEmployee, const QString& password) {
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);

    query.prepare("UPDATE PROFILES SET password = :password WHERE idEmployee = :id");
    query.bindValue(":password", password);
    query.bindValue(":id", idEmployee);

    if (!query.exec()) {
        qDebug() << "ERROR UPDATING PROFILE:" << query.lastError().text();
        return false;
    }

    qDebug() << "UPDATE PROFILE SUCCESSFUL";
    return true;
}

bool Profile_Model::checkIfUserExist(short int idEmployee, const QString& password) {
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);

    query.prepare("SELECT password FROM ACCOUNTS WHERE idEmployee = :id");
    query.bindValue(":id", idEmployee);
    if (!query.exec()) {
        qDebug() << "NO ACCOUNT FOUND:" << query.lastError().text();
        return false;
    }
    if (query.value("password") != password) {
        qDebug() << "WRONG PASSWORD";
        return false;
    }

    qDebug() << "UPDATE PROFILE SUCCESSFUL";
    return true;
}