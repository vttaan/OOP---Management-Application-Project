#ifndef KITCHENASSISTANT_H
#define KITCHENASSISTANT_H
#include "global.h"
#include "core/Staff.h"
class KitchenAssistant:public Staff{
private:
    double allowenceKitchen;
public:
    KitchenAssistant(short int idEmp,QString ava,QString idCit,QString n,QString d,QString add,QString phone , QString gender,int baseSalary,
bool isFixedEmployee, double allowence)
        :Staff("KitchenAssistant",idEmp,ava,idCit,n,d,add,phone,gender,baseSalary, isFixedEmployee), allowenceKitchen(allowence){}
    double getAllowence() const override { return allowenceKitchen; }
    void setAllowence() override {}
    void setAllowenceValue(double allowance) override { allowenceKitchen = allowance; }
    User* clone() const override { return new KitchenAssistant(*this); }
};
#endif // KITCHENASSISTANT_H
