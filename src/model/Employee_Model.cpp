#include "model/employee_model.h"

Employee_Model::Employee_Model() {}

void Employee_Model::loadData() {
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery query(openData);
    while(query.next()) {
        short curID = query.value("idEmployee").toInt();
        QString curRole = query.value("role").toString();
        QString curIdIndentity = query.value("IdCitizenIndentity").toString();
        QString curName = query.value("name").toString();
        QString curPhone = query.value("phoneNum").toString();
        QString curDob = query.value("dob").toString();
        QString curAddress = query.value("address").toString();
        QString curAvatarPath = query.value("avatarPath").toString();
        QString curGender = query.value("Gender").toString();
        User* nowEmployee = UserFactory::createContainsUser(curRole, curID, curAvatarPath, curIdIndentity, curName,
                                                            curDob, curAddress, curPhone, curGender);
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
    pattern = id + '|' + name + '|' + role + '|' + idIn + '|' + phone + '|' + dob + '|' + address + '|' + gender;
    return pattern;
}

bool Employee_Model::cmpAscending(User* a, User* b, vector<QString> contentSort) {
    for(int i = 0; i < contentSort.size(); i++) {
        if(a->getAnyAttributes(contentSort[i]) != b->getAnyAttributes(contentSort[i]))
            return a->getAnyAttributes(contentSort[i]) < b->getAnyAttributes(contentSort[i]);
    }
    return false;
}
bool Employee_Model::cmpDescending(User* a, User* b, vector<QString> contentSort) {
    for(int i = 0; i < contentSort.size(); i++) {
        if(a->getAnyAttributes(contentSort[i]) != b->getAnyAttributes(contentSort[i]))
            return a->getAnyAttributes(contentSort[i]) > b->getAnyAttributes(contentSort[i]);
    }
    return false;
}

vector<User*> Employee_Model::sortInEmployee(QString typeOrder, vector<QString> contentSort) {
    vector<User*> listOfSorted = this->listEmployee;
    if (typeOrder == "A-Z") {
        sort(listOfSorted.begin(), listOfSorted.end(), [this, contentSort](User* a, User* b) {
            return this->cmpAscending(a, b, contentSort);
        });
    }
    else {
        sort(listOfSorted.begin(), listOfSorted.end(), [this, contentSort](User* a, User* b) {
            return this->cmpDescending(a, b, contentSort);
        });
    }

    return listOfSorted;
}

bool Employee_Model::rabinKarp(QString pattern, QString contentSearch) {
    int n = pattern.size(), m = contentSearch.size();
    if(m > n) return false;
    int d = 256, q = 1e9+7, h = 1;
    for(int i = 1; i <= m; i++) h = (h * d) % q;

    int valHashOfPattern = 0, valHashOfContent = 0;
    for(int i = 0; i < m; i++) {
        valHashOfContent = (d * valHashOfContent + contentSearch[i].unicode()) % q;
        valHashOfPattern = (d * valHashOfPattern + pattern[i].unicode()) % q;
    }
    for(int i = m; i <= n; i++) {
        if(valHashOfContent == valHashOfPattern) {
            bool flag = true;
            for(int j = 0; j < m; j++) {
                if(contentSearch[j] != pattern[i + j]) {
                    flag = false;
                    break;
                }
            }
            if(flag == true) return true;
        }
        if(i < n - m) {
            valHashOfPattern = (d * (valHashOfPattern - pattern[i - m].unicode() * h) + pattern[i].unicode()) % q;
            if(valHashOfPattern < 0) valHashOfPattern += q;
        }
    }
    return false;
}

vector<User*> Employee_Model::filterInEmployee(vector<QString> contentFilter) {
    vector<User*> listOfFilter;
    for(int i = 0; i < this->listEmployee.size(); i++) {
        int cnt = 0;
        for(const QString& s: contentFilter) {
            if(this->listEmployee[i]->getRole() == s) cnt++;
            else if(this->listEmployee[i]->getGender() == s) cnt++;
        }
        if(cnt == contentFilter.size()) listOfFilter.push_back(this->listEmployee[i]);
    }
    return listOfFilter;
}

vector<User*> Employee_Model::searchInEmployee(QString contentSearch) {
    vector<User*> listOfEmployeeForSearch;
    for(int i = 0; i < this->listEmployee.size(); i++) {
        QString pattern = this->getPattern(i);
        if(this->rabinKarp(pattern, contentSearch)) listOfEmployeeForSearch.push_back(this->listEmployee[i]);
    }
    return listOfEmployeeForSearch;
}




