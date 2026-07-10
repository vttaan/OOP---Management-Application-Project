#include "global.h"
#include "Schedule_Control.h"

Schedule_Control::Schedule_Control(QObject *parent)
    : QObject(parent), view(nullptr) // Initialize view to nullptr for now, wait for team
{
    model = new Schedule_Model();
}

Schedule_Control::~Schedule_Control()
{
    delete model;
    // Note: view ownership depends on navigator/parent, typically not deleted here if managed globally
}

void Schedule_Control::load()
{
    qDebug() << "Schedule_Control: Loading shifts from database...";
    QList<Shift> loadedShifts;
    
    QSqlDatabase db = Database::getInstance()->getDbConnect();
    if (!db.isOpen()) {
        qDebug() << "Database is not open in Schedule_Control::load()!";
        return;
    }

    QSqlQuery query(db);
    // Fetch all shifts from the SHIFT table
    if (query.exec("SELECT IdShift, IdEmployee, workDate, startTime, endTime, status FROM SHIFT")) {
        while (query.next()) {
            short int idShift = query.value(0).toInt();
            short int idEmployee = query.value(1).toInt();
            QDate workDate = QDate::fromString(query.value(2).toString(), Qt::ISODate); // Assuming YYYY-MM-DD
            QTime startTime = QTime::fromString(query.value(3).toString(), "HH:mm"); // Assuming HH:mm
            QTime endTime = QTime::fromString(query.value(4).toString(), "HH:mm");
            // status is 5
            
            Shift newShift(idEmployee, workDate, startTime, endTime);
            // newShift could have status and idShift set here if setters existed
            
            loadedShifts.append(newShift);
        }
    } else {
        qDebug() << "Failed to execute load query for shifts:" << query.lastError().text();
    }
    
    model->setShifts(loadedShifts);
    qDebug() << "Schedule_Control: Successfully loaded" << loadedShifts.size() << "shifts.";
}

void Schedule_Control::handleSaveSchedule()
{
    qDebug() << "Schedule_Control: handleSaveSchedule called.";
    // Logic to save any modified shifts in the model back to the database
}

void Schedule_Control::handleGenSchedule()
{
    // Leave blank for now as requested
}

void Schedule_Control::search()
{
    qDebug() << "Schedule_Control: search called.";
}

void Schedule_Control::filter()
{
    qDebug() << "Schedule_Control: filter called.";
}

void Schedule_Control::chooseDate()
{
    qDebug() << "Schedule_Control: chooseDate called.";
}

void Schedule_Control::handleChangeAlgorithm()
{
    // Leave blank for now as requested
}

void Schedule_Control::setView(Schedule_View* view)
{
    this->view = view;
}

Schedule_View* Schedule_Control::getView() const
{
    return this->view;
}
