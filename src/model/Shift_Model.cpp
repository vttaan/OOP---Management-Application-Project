#include "Shift_Model.h"

Shift_Model::Shift_Model() {}


bool Shift_Model::checkOverlapping(short int id, QDate date, QTime start, QTime end) {
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    QSqlQuery queryAccount(openData);
    queryAccount.prepare("SELECT * FROM ACCOUNTS");
}