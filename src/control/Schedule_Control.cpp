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
