#include "global.h"
#include "User.h"

User::User(QString r, short int idEmp, QString ava,
    QString idCit, QString n, QString d, QString add, QString phone, QString gender)
    : role(r), idEmployee(idEmp),
    avatarPath(ava), idCitizenIdentity(idCit), name(n), dob(d),
    address(add), phoneNum(phone), gender(gender) {
}

QString User::getRole() const { return role; }
short int User::getIdEmployee() const { return idEmployee; }
QString User::getIdentityID() const { return idCitizenIdentity; }
QString User::getName() const { return name; }
QString User::getAddress() const { return address; }
QString User::getDOB() const { return dob; }
QString User::getAvatarPath() const { return avatarPath; }
QString User::getPhoneNum() const { return phoneNum; }
QString User::getGender() const {return this->gender;}
QString User::getAnyAttributes(QString content) const {
    QString check = content.toUpper();
    if(check == "ROLE") return this->getRole();
    else if(check == "ID") return QString::number(this->getIdEmployee());
    else if(check == "NAME") return this->getName();
    else if(check == "ADDRESS") return this->getAddress();
    else if(check == "DATE OF BIRTH") return this->getDOB();
    else if(check == "PHONE") return this->getPhoneNum();
    else if(check == "GENDER") return this->getGender();
    else if(check == "ID_INDENTITY") return this->getIdentityID();
    return "";
}
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