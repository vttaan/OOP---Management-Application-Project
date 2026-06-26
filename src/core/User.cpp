#include "User.h"

User::User(QString r, short int idEmp, QString ava,
    QString idCit, QString n, QString d, QString add, QString phone)
    : role(r), idEmployee(idEmp),
    avatarPath(ava), idCitizenIdentity(idCit), name(n), dob(d),
    address(add), phoneNum(phone) {
}

QString User::getRole() const { return role; }
short int User::getIdEmployee() const { return idEmployee; }
QString User::getIndentityID() const { return idCitizenIdentity; }
QString User::getName() const { return name; }
QString User::getAddress() const { return address; }
QString User::getDOB() const { return dob; }
QString User::getAvatarPath() const { return avatarPath; }
QString User::getPhoneNum() const { return phoneNum; }

void User::setRole(QString r) {
    this->role = r;
}
void User::setIdEmployee(short int id) {
    this->idEmployee = id;
}
void User::setIndentityID(QString idCit) {
    this->idCitizenIdentity = idCit;
}
void User::setName(QString n) {
    this->name = n;
}
void User::setAddress(QString add) {
    this->address = add;
}
void User::setDOB(QString d) {
    this->dob = d;
}
void User::setAva(QString a) {
    this->avatarPath = a;
}
void User::setPhoneNum(QString phone) {
    this->phoneNum = phone;
}