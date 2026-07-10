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
    // Check if the given time range overlaps with any existing shift for the same employee on the same date
    for (const Shift& shift : currentShifts) {
        if (shift.getEmployeeID() == id && shift.getDate() == date) {
            // Overlap condition: (StartA < EndB) and (EndA > StartB)
            if (start < shift.getEndTime() && end > shift.getStartTime()) {
                return true; // Overlap found
            }
        }
    }
    return false; // No overlap
}
