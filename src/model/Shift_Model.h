#ifndef SHIFT_MODEL_H
#define SHIFT_MODEL_H
#include "core/Shift.h"
#include <QString>
#include "utils/Database.h"


class Shift_Model
{
public:
    Shift_Model();
    bool checkOverlapping(short int id, QDate date, QTime start, QTime end);
};

#endif // SHIFT_MODEL_H
