#include "global.h"
#include "Dashboard_Control.h"
#include "view/Dashboard_View.h"
#include "view/employeecard.h"

Dashboard_Control::Dashboard_Control(QObject *parent)
    :QObject(parent), view(nullptr), currentSession(SessionManager::getInstance()), empModel(new Employee_Model()){

}
Dashboard_Control::~Dashboard_Control() {
    // view is owned by View_Navigator, do not delete here
    // currentSession is owned by Control_Navigator, do not delete here
    delete empModel;
}
void Dashboard_Control::init(){
    if(!view)return;
    empModel->loadData();
    QList<User*>all=empModel->getListEmployee();
    int totalStaff=0;
    int totalManager=0;
    for(User*u:all){
        if(u->getRole().contains("Manager",Qt::CaseInsensitive)) totalManager++;
        else totalStaff++;
    }
    int workingToday=0;
    {
        QSqlQuery q(Database::getInstance()->getDbConnect());
        q.prepare("SELECT COUNT(DISTINCT idEmployee) FROM SHIFT "
                  "WHERE workDate = :today AND status = 1");
        q.bindValue(":today", QDate::currentDate().toString(Qt::ISODate));
        if (q.exec() && q.next()) workingToday = q.value(0).toInt();
    }
    view->updateStatCards(totalStaff+totalManager,totalStaff,totalManager,workingToday);
    loadEmployeeCards(all);
    loadShiftPanel();
}

Dashboard_View* Dashboard_Control::getView()  {
    return this->view;
}
void Dashboard_Control::setView(Dashboard_View* view) {
    this->view = view;
    if (!this->view) return;
    QObject::connect(this->view, &Dashboard_View::profileClicked,
                     this, &Dashboard_Control::profilePageClicked);
    QObject::connect(this->view,&Dashboard_View::searchChanged,this,&Dashboard_Control::onSearchChanged);
}
void Dashboard_Control::loadEmployeeCards(const QList<User *> &list){
    view->clearEmployeeGrid();
    QSet<int>workingIds;
    {
        QSqlQuery q(Database::getInstance()->getDbConnect());
        q.prepare("SELECT DISTINCT idEmployee FROM SHIFT "
                  "WHERE workDate = :today AND status = 1");
        q.bindValue(":today", QDate::currentDate().toString(Qt::ISODate));
        if (q.exec()) while (q.next()) workingIds.insert(q.value(0).toInt());
    }
    for(User*u:list){
        EmployeeCard*card=new EmployeeCard();
        QString fakeEmail = QString("nv%1@congty.com").arg(u->getIdEmployee());
        card->setData(u->getAvatarPath(),u->getName(),u->getRole(),fakeEmail,u->getPhoneNum(),
                      QString::number(u->getIdEmployee()), u->getDOB(), u->getGender());
        card->setStatus(workingIds.contains(u->getIdEmployee()));
        view->addEmployeeCard(card);
    }
}
void Dashboard_Control::loadShiftPanel(){
    QList<QPair<QString,QString>> nextShifts;
    {
        QSqlQuery q(Database::getInstance()->getDbConnect());
        q.prepare(
            "SELECT P.name, S.startTime, S.endTime FROM SHIFT S "
            "JOIN PROFILES P ON S.idEmployee = P.idEmployee "
            "WHERE S.workDate = :today AND S.startTime > :now AND S.status = 1 "
            "ORDER BY S.startTime LIMIT 5");
        q.bindValue(":today", QDate::currentDate().toString(Qt::ISODate));
        q.bindValue(":now",   QTime::currentTime().toString("HH:mm"));
        if (q.exec())
            while (q.next())
                nextShifts.append({q.value(0).toString(),
                                   q.value(1).toString() + " - " + q.value(2).toString()});
    }

    QStringList absentNames;
    {
        QSqlQuery q(Database::getInstance()->getDbConnect());
        q.prepare(
            "SELECT P.name, 'Nghỉ phép' AS reason "
            "FROM PROFILES P "
            "JOIN LEAVE_REQUEST L ON P.idEmployee = L.idEmployee "
            "WHERE L.leaveDate = :today AND L.status = 'Approved' "
            
            "UNION "
            
            "SELECT P.name, 'Vắng mặt' AS reason "
            "FROM PROFILES P "
            "JOIN SHIFT S ON P.idEmployee = S.idEmployee "
            "WHERE S.workDate = :today AND S.status = 1 AND S.startTime <= :now "
            "AND P.idEmployee NOT IN ("
            "    SELECT idEmployee FROM TIMEKEEPING WHERE checkInDate = :today"
            ")"
        );
        q.bindValue(":today", QDate::currentDate().toString(Qt::ISODate));
        q.bindValue(":now", QTime::currentTime().toString("HH:mm"));
        
        if (q.exec()) {
            while (q.next()) {
                QString name = q.value(0).toString();
                QString reason = q.value(1).toString();
                absentNames << QString("%1 (%2)").arg(name, reason); 
            }
        }
    }
    view->updateShiftPanel(nextShifts,absentNames);
}
void Dashboard_Control::onSearchChanged(const QString& text){
    QList<User*> filtered = empModel->SearchSortFilter(
        text, 0, {}, {});
    loadEmployeeCards(filtered);
}