#include "global.h"
#include "model/Employee_Model.h"
#include "utils/Database.h"
#include "utils/Security.h"

Employee_Model::Employee_Model() {}


int Employee_Model::getNextId(const QString& role) {
    QSqlQuery query;
    int newId = 1;

    query.prepare("SELECT MAX(idEmployee) AS MaxID FROM PROFILES WHERE role = :u");
    query.bindValue(":u", role);

    if (query.exec() && query.next()) {
        QVariant v = query.value("MaxID");
        if (!v.isNull()) {
            newId = v.toInt() + 1;
        }
    }
    return newId;
}
bool Employee_Model::addUserInList(User *emp)
{
  if (emp == nullptr)
    return false;
  this->listEmployee.append(emp);
  return true;
}

bool Employee_Model::popUserInList(short idEmployee)
{
  if (this->listEmployee.size() < 1)
    return false;
  for (int i = 0; i < this->listEmployee.size(); i++)
  {
    if (this->listEmployee[i]->getIdEmployee() == idEmployee)
    {
      delete this->listEmployee.takeAt(i);
      return true;
    }
  }
  return false;
}

bool Employee_Model::addEmployee(const QString &role, const QString &avatarPath, const QString &citizenId,
                                 const QString &name, const QString &dob, const QString &address,
                                 const QString &phone, const QString &gender,
                                 const QString &username, const QString &password)
{
  User *emp = UserFactory::createNewUser(role, avatarPath, citizenId, name, dob, address, phone, gender);
    if (emp == nullptr) {
      qDebug() << "Create fail\n";
        return false;
    }
    //return false;
  QSqlDatabase db = Database::getInstance()->getDbConnect();
  db.transaction(); // start transaction before inserting

  // Copy avatar to local folder and update user object
  QString localAva =
      saveAvatarLocally(emp->getIdEmployee(), emp->getAvatarPath());
  qDebug() << emp->getIdEmployee() << '\n';
  emp->setAva(localAva);

  QSqlQuery qProfile(db);
  qProfile.prepare(
      "INSERT INTO PROFILES (idEmployee, role, name, phoneNum, dob, address, "
      "avatarPath, IdCitizenIdentity, Gender) "
      "VALUES (:id,:role,:name,:phone,:dob,:address,:avatar,:citizen, :Gender)");
  qProfile.bindValue(":id", emp->getIdEmployee());
  qProfile.bindValue(":role", emp->getRole());
  qProfile.bindValue(":name", emp->getName());
  qProfile.bindValue(":phone", emp->getPhoneNum());
  qProfile.bindValue(":dob", emp->getDOB());
  qProfile.bindValue(":address", emp->getAddress());
  qProfile.bindValue(":avatar", localAva);
  qProfile.bindValue(":citizen", emp->getIdentityID());
  qProfile.bindValue(":Gender", emp->getGender());
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
    qAccount.prepare("INSERT INTO ACCOUNTS (userName, passWord, idEmployee) "
                     "VALUES (:user, :pass, :id)");
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
  if (!this->addUserInList(emp))
    qDebug() << "ADD USER IN LIST FAILED, POINTER USER IS NULL\n";
  else
    qDebug() << "ADD USER IN LIST SUCCESS\n"; // add in list, uses for sort, search, filter.
  return db.commit();
}

bool Employee_Model::updateEmployee(User *emp)
{
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
                "address = :address, avatarPath = :avatar, IdCitizenIdentity "
                "= :citizen, Gender = :gender WHERE idEmployee = :id");
  query.bindValue(":role", emp->getRole());
  query.bindValue(":name", emp->getName());
  query.bindValue(":phone", emp->getPhoneNum());
  query.bindValue(":dob", emp->getDOB());
  query.bindValue(":address", emp->getAddress());
  query.bindValue(":avatar", emp->getAvatarPath());
  query.bindValue(":citizen", emp->getIdentityID());
  query.bindValue(":id", emp->getIdEmployee());
  query.bindValue(":gender", emp->getGender());

  if (!query.exec())
  {
    qDebug() << "Error updating profile:" << query.lastError().text();
    return false;
  }
  return true;
}

bool Employee_Model::deleteEmployee(short idEmployee)
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
  if (!this->popUserInList(idEmployee))
    qDebug() << "POP USER IN LIST FAILDE, LIST IS EMPTY\n";
  return db.commit();
}

QString Employee_Model::saveAvatarLocally(int empId,
                                          const QString &sourcePath)
{
  if (sourcePath.isEmpty())
    return "";

  // If it's already a resource path, do not copy
  if (sourcePath.startsWith(":/"))
    return sourcePath;

  QFileInfo sourceInfo(sourcePath);
  if (!sourceInfo.exists())
    return "";
  QDir appDir = QCoreApplication::applicationDirPath(); // debug folder
  appDir.cdUp();                                        // build folder
  appDir.cdUp();                                        // MAP folder
  // Create "avatars" directory in application directory if not exists
  QString targetDir = appDir.filePath("resources") + "/avatars"; // avatars folder in resouces folder
  QDir dir(targetDir);
  if (!dir.exists())
  {
    dir.mkpath(".");
  }

  // Generate path: appDir/avatars/avatar_X.ext
  QString ext = sourceInfo.suffix().toLower();
  if (ext.isEmpty())
    ext = "png";
  QString targetPath =
      QString("%1/avatar_%2.%3").arg(targetDir).arg(empId).arg(ext);

  // If file already exists, remove it first to overwrite
  if (QFile::exists(targetPath))
  {
    QFile::remove(targetPath);
  }

  if (QFile::copy(sourcePath, targetPath))
  {
    return QString("avatar_%1.%2").arg(empId).arg(ext);
  }

  return sourcePath;
}

void Employee_Model::loadData()
{
  if (this->listEmployee.empty() == false)
    return;
  if (!this->listEmployee.empty())
  {
    qDeleteAll(listEmployee);
    listEmployee.clear();
  }
  QSqlDatabase openData = Database::getInstance()->getDbConnect();
  QSqlQuery query("SELECT * FROM PROFILES", openData);
  while (query.next())
  {
    short curID = query.value("idEmployee").toInt();
    QString curRole = query.value("role").toString();
    QString curIdIndentity = query.value("IdCitizenIdentity").toString();
    QString curName = query.value("name").toString();
    QString curPhone = query.value("phoneNum").toString();
    QString curDob = query.value("dob").toString();
    QString curAddress = query.value("address").toString();
    QString curAvatarPath = query.value("avatarPath").toString();
    QString curGender = query.value("Gender").toString();
    User *nowEmployee = UserFactory::createContainsUser(
        curRole, curID, curAvatarPath, curIdIndentity, curName, curDob,
        curAddress, curPhone, curGender);
    this->listEmployee.append(nowEmployee);
  }
}

QList<User *> Employee_Model::SearchSortFilter(QString contentSearch, short typeOrder, QList<QString> contentSort,
                                               QList<QString> contentFilter)
{
  QList<User *> result = this->listEmployee;
  // search
  if (!contentSearch.isEmpty())
    result = this->searchInEmployee(result, contentSearch);
  // sort
  if (!contentSort.isEmpty())
    result = this->sortInEmployee(result, typeOrder, contentSort);
  // filter
  if (!contentFilter.isEmpty())
    result = this->filterInEmployee(result, contentFilter);

  return result;
}

//----------METHOD SUPPORT FOR SEACRH, FILTER, SORT--------------------------------

QString Employee_Model::removeAccent(const QString &input)
{
  QString res = "";
  QMap<QString, QString> mapping =
      {
          {"à", "a"}, {"á", "a"}, {"ạ", "a"}, {"ả", "a"}, {"ã", "a"}, {"â", "a"}, {"ầ", "a"}, {"ấ", "a"}, {"ậ", "a"}, {"ẩ", "a"}, {"ẫ", "a"}, {"ă", "a"}, {"ằ", "a"}, {"ắ", "a"}, {"ặ", "a"}, {"ẳ", "a"}, {"ẵ", "a"}, {"è", "e"}, {"é", "e"}, {"ẹ", "e"}, {"ẻ", "e"}, {"ẽ", "e"}, {"ê", "e"}, {"ề", "e"}, {"ế", "e"}, {"ệ", "e"}, {"ể", "e"}, {"ễ", "e"}, {"ì", "i"}, {"í", "i"}, {"ị", "i"}, {"ỉ", "i"}, {"ĩ", "i"}, {"ò", "o"}, {"ó", "o"}, {"ọ", "o"}, {"ỏ", "o"}, {"õ", "o"}, {"ô", "o"}, {"ồ", "o"}, {"ố", "o"}, {"ộ", "o"}, {"ổ", "o"}, {"ỗ", "o"}, {"ơ", "o"}, {"ờ", "o"}, {"ớ", "o"}, {"ợ", "o"}, {"ở", "o"}, {"ỡ", "o"}, {"ù", "u"}, {"ú", "u"}, {"ụ", "u"}, {"ủ", "u"}, {"ũ", "u"}, {"ư", "u"}, {"ừ", "u"}, {"ứ", "u"}, {"ự", "u"}, {"ử", "u"}, {"ữ", "u"}, {"ỳ", "y"}, {"ý", "y"}, {"ỵ", "y"}, {"ỷ", "y"}, {"ỹ", "y"}, {"đ", "d"}};

  for (QChar c : input)
  {
    if (mapping.count(c))
      res += mapping[c];
    else
      res += c;
  }

  return res;
}

QString Employee_Model::getPattern(User *emp)
{
  QString pattern = "";
  QString id = QString::number(emp->getIdEmployee());
  QString name = emp->getName();
  QString role = emp->getRole();
  QString idIn = emp->getIdentityID();
  QString phone = emp->getPhoneNum();
  QString dob = emp->getDOB();
  QString address = emp->getAddress();
  QString gender = emp->getGender();
  pattern = id + '|' + name + '|' + role + '|' + idIn + '|' + phone + '|' +
            dob + '|' + address + '|' + gender;
  QString patternNew = pattern.toLower();
  patternNew = this->removeAccent(patternNew);
  return patternNew;
}

bool Employee_Model::cmpAscending(User *a, User *b,
                                  QList<QString> contentSort)
{
  for (int i = 0; i < contentSort.size(); i++)
  {
    if (a->getAnyAttributes(contentSort[i]) !=
        b->getAnyAttributes(contentSort[i]))
      return a->getAnyAttributes(contentSort[i]) <
             b->getAnyAttributes(contentSort[i]);
  }
  return false;
}
bool Employee_Model::cmpDescending(User *a, User *b,
                                   QList<QString> contentSort)
{
  for (int i = 0; i < contentSort.size(); i++)
  {
    if (a->getAnyAttributes(contentSort[i]) !=
        b->getAnyAttributes(contentSort[i]))
      return a->getAnyAttributes(contentSort[i]) >
             b->getAnyAttributes(contentSort[i]);
  }
  return false;
}

QList<User *> Employee_Model::sortInEmployee(QList<User *> inputList, short typeOrder,
                                             QList<QString> contentSort)
{
  QList<User *> listOfSorted = inputList;
  if (typeOrder == 1)
  {
    stable_sort(listOfSorted.begin(), listOfSorted.end(),
                [this, contentSort](User *a, User *b)
                {
                  return this->cmpAscending(a, b, contentSort);
                });
  }
  else
  {
    stable_sort(listOfSorted.begin(), listOfSorted.end(),
                [this, contentSort](User *a, User *b)
                {
                  return this->cmpDescending(a, b, contentSort);
                });
  }

  return listOfSorted;
}

bool Employee_Model::rabinKarp(QString pattern, QString contentSearch)
{
  int n = pattern.size(), m = contentSearch.size();
  if (m > n)
    return false;
  long long d = 256, q = 1e9 + 7, h = 1;
  for (int i = 0; i < m - 1; i++)
    h = (h * d) % q;

  int valHashOfPattern = 0, valHashOfContent = 0;
  for (int i = 0; i < m; i++)
  {
    valHashOfContent = (d * valHashOfContent + contentSearch[i].unicode()) % q;
    valHashOfPattern = (d * valHashOfPattern + pattern[i].unicode()) % q;
  }
  for (int i = 0; i <= n - m; i++)
  {
    if (valHashOfContent == valHashOfPattern)
    {
      bool flag = true;
      for (int j = 0; j < m; j++)
      {
        if (contentSearch[j] != pattern[i + j])
        {
          flag = false;
          break;
        }
      }
      if (flag == true)
        return true;
    }
    if (i < n - m)
    {
      valHashOfPattern = (d * (valHashOfPattern - pattern[i].unicode() * h) + pattern[i + m].unicode()) % q;
      if (valHashOfPattern < 0)
        valHashOfPattern += q;
    }
  }
  return false;
}

QList<User *> Employee_Model::filterInEmployee(QList<User *> inputList, QList<QString> contentFilter)
{
  QList<User *> listOfFilter;
  for (int i = 0; i < inputList.size(); i++)
  {
    int cnt = 0;
    for (const QString &s : contentFilter)
    {
      if (inputList[i]->getRole() == s)
        cnt++;
      else if (inputList[i]->getGender() == s)
        cnt++;
    }
    if (cnt == contentFilter.size())
      listOfFilter.push_back(inputList[i]);
  }
  return listOfFilter;
}

QList<User *> Employee_Model::searchInEmployee(QList<User *> inputList, QString contentSearch)
{
  QList<User *> listOfEmployeeForSearch;

  contentSearch = contentSearch.toLower();
  contentSearch = this->removeAccent(contentSearch);

  for (int i = 0; i < inputList.size(); i++)
  {
    QString pattern = this->getPattern(inputList[i]);
    if (this->rabinKarp(pattern, contentSearch))
      listOfEmployeeForSearch.push_back(inputList[i]);
  }
  return listOfEmployeeForSearch;
}

QString Employee_Model::generateAutoUsername(int id, QString& role){
    QString res = (role=="Staff")? "NV_" : "QL_";
    res += QString::number(id).rightJustified(3, '0');
    // can memories 3 number 0 leading.
    return res;
}

QString Employee_Model::generateAutoPassword(QString &name, QString &dob){
    QStringList words = name.split(" ", Qt::SkipEmptyParts);
    QString res = "";

    for(const QString& w : words){
        res += w.at(0).toUpper();
    }
    // yyyy-mm-dd
    QStringList dates = dob.split("-");


    QString dd = dates[2], mm = dates[1];

    return res + '#' + dd + mm;
    // TQT#0609
}