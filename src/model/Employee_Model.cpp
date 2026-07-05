#include "model/Employee_Model.h"
#include "utils/Database.h"
#include "utils/Security.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>


Employee_Model::Employee_Model() {}

QList<User *> Employee_Model::getAllEmployees() {
  QList<User *> list;

  QSqlDatabase db = Database::getInstance()->getDbConnect();
  QSqlQuery query("SELECT * FROM PROFILES",
                  db); // Fixed: QSqlQuery (was QSqlDatabase)

  while (query.next()) {
    short int id = query.value("idEmployee").toInt();
    QString role = query.value("role").toString();
    QString name = query.value("name").toString();
    QString phone = query.value("phoneNum").toString();
    QString dob = query.value("dob").toString();
    QString address = query.value("address").toString();
    QString avatar = query.value("avatarPath").toString();
    QString citizenId = query.value("IdCitizenIndentity").toString();
    QString curGender = query.value("Gender").toString();

    User *emp = UserFactory::createContainsUser(
        role, id, avatar, citizenId, name, dob, address, phone,
        curGender); // Fixed: citizenID → citizenId
    if (emp) {
      list.append(emp);
    }
  }
  return list;
}

bool Employee_Model::addEmployee(User *emp, const QString &username,
                                 const QString &password) {
  QSqlDatabase db = Database::getInstance()->getDbConnect();
  db.transaction(); // start transaction before inserting

  // Copy avatar to local folder and update user object
  QString localAva =
      saveAvatarLocally(emp->getIdEmployee(), emp->getAvatarPath());
  emp->setAva(localAva);

  QSqlQuery qProfile(db);
  qProfile.prepare(
      "INSERT INTO PROFILES (idEmployee, role, name, phoneNum, dob, address, "
      "avatarPath, IdCitizenIndentity) "
      "VALUES (:id,:role,:name,:phone,:dob,:address,:avatar,:citizen)");
  qProfile.bindValue(":id", emp->getIdEmployee());
  qProfile.bindValue(":role", emp->getRole());
  qProfile.bindValue(":name", emp->getName());
  qProfile.bindValue(":phone", emp->getPhoneNum());
  qProfile.bindValue(":dob", emp->getDOB());
  qProfile.bindValue(":address", emp->getAddress());
  qProfile.bindValue(":avatar", emp->getAvatarPath());
  qProfile.bindValue(":citizen", emp->getIdentityID());

  if (!qProfile.exec()) {
    qDebug() << "Error adding profile" << qProfile.lastError().text();
    db.rollback();
    return false;
  }

  if (!username.isEmpty() && !password.isEmpty()) {
    QSqlQuery qAccount(db);
    QString hashedPass = Security::hashPassword(password);
    qAccount.prepare("INSERT INTO ACCOUNTS (userName, passWord, idEmployee) "
                     "VALUES (:user, :pass, :id)");
    qAccount.bindValue(":user", username);
    qAccount.bindValue(":pass", hashedPass);
    qAccount.bindValue(":id", emp->getIdEmployee());

    if (!qAccount.exec()) {
      qDebug() << "Error adding account:" << qAccount.lastError().text();
      db.rollback();
      return false;
    }
  }

  return db.commit();
}

bool Employee_Model::updateEmployee(User *emp) {
  if (!emp)
    return false;

  // Copy avatar to local folder and update user object
  QString localAva =
      saveAvatarLocally(emp->getIdEmployee(), emp->getAvatarPath());
  emp->setAva(localAva);

  QSqlDatabase db = Database::getInstance()->getDbConnect();
  QSqlQuery query(db);
  query.prepare("UPDATE PROFILES SET role = :role, name = :name, phoneNum = "
                ":phone, dob = :dob, "
                "address = :address, avatarPath = :avatar, IdCitizenIndentity "
                "= :citizen WHERE idEmployee = :id");
  query.bindValue(":role", emp->getRole());
  query.bindValue(":name", emp->getName());
  query.bindValue(":phone", emp->getPhoneNum());
  query.bindValue(":dob", emp->getDOB());
  query.bindValue(":address", emp->getAddress());
  query.bindValue(":avatar", emp->getAvatarPath());
  query.bindValue(":citizen", emp->getIdentityID());
  query.bindValue(":id", emp->getIdEmployee());

  if (!query.exec()) {
    qDebug() << "Error updating profile:" << query.lastError().text();
    return false;
  }
  return true;
}

bool Employee_Model::deleteEmployee(int idEmployee) {
  QSqlDatabase db = Database::getInstance()->getDbConnect();
  db.transaction();

  QSqlQuery qAccount(db);
  qAccount.prepare("DELETE FROM ACCOUNTS WHERE idEmployee = :id");
  qAccount.bindValue(":id", idEmployee);
  qAccount.exec();

  QSqlQuery qProfile(db);
  qProfile.prepare("DELETE FROM PROFILES WHERE idEmployee = :id");
  qProfile.bindValue(":id", idEmployee);

  if (!qProfile.exec()) {
    qDebug() << "Error deleting profile:" << qProfile.lastError().text();
    db.rollback();
    return false;
  }

  return db.commit();
}

QString Employee_Model::saveAvatarLocally(int empId,
                                          const QString &sourcePath) {
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
  if (!dir.exists()) {
    dir.mkpath(".");
  }

  // Generate path: appDir/avatars/avatar_X.ext
  QString ext = sourceInfo.suffix().toLower();
  if (ext.isEmpty())
    ext = "png";
  QString targetPath =
      QString("%1/avatar_%2.%3").arg(targetDir).arg(empId).arg(ext);

  // If file already exists, remove it first to overwrite
  if (QFile::exists(targetPath)) {
    QFile::remove(targetPath);
  }

  if (QFile::copy(sourcePath, targetPath)) {
    return targetPath;
  }

  return sourcePath;
}

void Employee_Model::loadData() {
  QSqlDatabase openData = Database::getInstance()->getDbConnect();
  QSqlQuery query(openData);
  while (query.next()) {
    short curID = query.value("idEmployee").toInt();
    QString curRole = query.value("role").toString();
    QString curIdIndentity = query.value("IdCitizenIndentity").toString();
    QString curName = query.value("name").toString();
    QString curPhone = query.value("phoneNum").toString();
    QString curDob = query.value("dob").toString();
    QString curAddress = query.value("address").toString();
    QString curAvatarPath = query.value("avatarPath").toString();
    QString curGender = query.value("Gender").toString();
    User *nowEmployee = UserFactory::createContainsUser(
        curRole, curID, curAvatarPath, curIdIndentity, curName, curDob,
        curAddress, curPhone, curGender);
    this->listEmployee.push_back(nowEmployee);
  }
}

QString Employee_Model::getPattern(short i) {
  QString pattern = "";
  QString id = QString::number(this->listEmployee[i]->getIdEmployee());
  QString name = this->listEmployee[i]->getName();
  QString role = this->listEmployee[i]->getRole();
  QString idIn = this->listEmployee[i]->getIdentityID();
  QString phone = this->listEmployee[i]->getPhoneNum();
  QString dob = this->listEmployee[i]->getDOB();
  QString address = this->listEmployee[i]->getAddress();
  QString gender = this->listEmployee[i]->getGender();
  pattern = id + '|' + name + '|' + role + '|' + idIn + '|' + phone + '|' +
            dob + '|' + address + '|' + gender;
  return pattern;
}

bool Employee_Model::cmpAscending(User *a, User *b,
                                  vector<QString> contentSort) {
  for (int i = 0; i < contentSort.size(); i++) {
    if (a->getAnyAttributes(contentSort[i]) !=
        b->getAnyAttributes(contentSort[i]))
      return a->getAnyAttributes(contentSort[i]) <
             b->getAnyAttributes(contentSort[i]);
  }
  return false;
}
bool Employee_Model::cmpDescending(User *a, User *b,
                                   vector<QString> contentSort) {
  for (int i = 0; i < contentSort.size(); i++) {
    if (a->getAnyAttributes(contentSort[i]) !=
        b->getAnyAttributes(contentSort[i]))
      return a->getAnyAttributes(contentSort[i]) >
             b->getAnyAttributes(contentSort[i]);
  }
  return false;
}

vector<User *> Employee_Model::sortInEmployee(QString typeOrder,
                                              vector<QString> contentSort) {
  vector<User *> listOfSorted = this->listEmployee;
  if (typeOrder == "A-Z") {
    sort(listOfSorted.begin(), listOfSorted.end(),
         [this, contentSort](User *a, User *b) {
           return this->cmpAscending(a, b, contentSort);
         });
  } else {
    sort(listOfSorted.begin(), listOfSorted.end(),
         [this, contentSort](User *a, User *b) {
           return this->cmpDescending(a, b, contentSort);
         });
  }

  return listOfSorted;
}

bool Employee_Model::rabinKarp(QString pattern, QString contentSearch) {
  int n = pattern.size(), m = contentSearch.size();
  if (m > n)
    return false;
  int d = 256, q = 1e9 + 7, h = 1;
  for (int i = 1; i <= m; i++)
    h = (h * d) % q;

  int valHashOfPattern = 0, valHashOfContent = 0;
  for (int i = 0; i < m; i++) {
    valHashOfContent = (d * valHashOfContent + contentSearch[i].unicode()) % q;
    valHashOfPattern = (d * valHashOfPattern + pattern[i].unicode()) % q;
  }
  for (int i = m; i <= n; i++) {
    if (valHashOfContent == valHashOfPattern) {
      bool flag = true;
      for (int j = 0; j < m; j++) {
        if (contentSearch[j] != pattern[i + j]) {
          flag = false;
          break;
        }
      }
      if (flag == true)
        return true;
    }
    if (i < n - m) {
      valHashOfPattern =
          (d * (valHashOfPattern - pattern[i - m].unicode() * h) +
           pattern[i].unicode()) %
          q;
      if (valHashOfPattern < 0)
        valHashOfPattern += q;
    }
  }
  return false;
}

vector<User *> Employee_Model::filterInEmployee(vector<QString> contentFilter) {
  vector<User *> listOfFilter;
  for (int i = 0; i < this->listEmployee.size(); i++) {
    int cnt = 0;
    for (const QString &s : contentFilter) {
      if (this->listEmployee[i]->getRole() == s)
        cnt++;
      else if (this->listEmployee[i]->getGender() == s)
        cnt++;
    }
    if (cnt == contentFilter.size())
      listOfFilter.push_back(this->listEmployee[i]);
  }
  return listOfFilter;
}

vector<User *> Employee_Model::searchInEmployee(QString contentSearch) {
  vector<User *> listOfEmployeeForSearch;
  for (int i = 0; i < this->listEmployee.size(); i++) {
    QString pattern = this->getPattern(i);
    if (this->rabinKarp(pattern, contentSearch))
      listOfEmployeeForSearch.push_back(this->listEmployee[i]);
  }
  return listOfEmployeeForSearch;
}