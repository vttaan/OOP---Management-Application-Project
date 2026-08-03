#ifndef HALLSTAFF_H
#define HALLSTAFF_H
#include "global.h"
#include "core/Staff.h"
class HallStaff:public Staff{
private:
    double allowenceHall;
public:
    HallStaff(short int idEmp,QString ava,QString idCit,QString n,QString d,QString add,QString phone,QString gender,int baseSalary,
              bool isFixedEmployee, double allowence)
        :Staff("HallStaff",idEmp,ava,idCit,n,d,add,phone ,gender,baseSalary, isFixedEmployee), allowenceHall(allowence){}
    double getAllowence() const override;
    void setAllowence() override;
     void setAllowenceValue(double allowance) override { allowenceHall = allowance; }
    User* clone() const override { return new HallStaff(*this); }
};

#endif // HALLSTAFF_H
