#ifndef HALLSTAFF_H
#define HALLSTAFF_H
#include "global.h"
#include "core/Staff.h"
class HallStaff:public Staff{
public:
    HallStaff(short int idEmp,QString ava,QString idCit,QString n,QString d,QString add,QString phone,QString gender,int baseSalary):Staff("HallStaff",idEmp,ava,idCit,n,d,add,phone ,gender,baseSalary){}

};

#endif // HALLSTAFF_H
