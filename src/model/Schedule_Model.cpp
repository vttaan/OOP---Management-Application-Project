#include "global.h"
#include "Schedule_Model.h"

Schedule_Model::Schedule_Model() {}

void Schedule_Model::setShifts(const QList<Shift>& shifts) {
    this->currentShifts = shifts;
}

QList<Shift> Schedule_Model::getShifts() const {
    return this->currentShifts;
}

bool Schedule_Model::checkOverlapping(short int id, QDate date, QTime start, QTime end) {
    QSqlDatabase openData = Database::getInstance()->getDbConnect();
    if (!openData.isOpen()) {
        qDebug() << "Database is not open in checkOverlapping!";
        return false;
    }
    
    QSqlQuery query(openData);
    query.prepare("SELECT * FROM SHIFT WHERE IdEmployee = :id AND workDate = :date");
    query.bindValue(":id", id);
    query.bindValue(":date", date.toString(Qt::ISODate));
    
    if (query.exec()) {
        while (query.next()) {
            QTime existingStart = QTime::fromString(query.value("startTime").toString(), "HH:mm");
            QTime existingEnd = QTime::fromString(query.value("endTime").toString(), "HH:mm");
            
            // Overlap condition: (StartA < EndB) and (EndA > StartB)
            if (start < existingEnd && end > existingStart) {
                return true; // Overlap found
            }
        }
    } else {
        qDebug() << "checkOverlapping query failed:" << query.lastError().text();
    }
    
    return false; // No overlap
}
