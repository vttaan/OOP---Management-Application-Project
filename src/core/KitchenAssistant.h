#ifndef KITCHENASSISTANT_H
#define KITCHENASSISTANT_H
#include "global.h"
#include "core/Staff.h"
class KitchenAssistant:public Staff{
public:
    KitchenAssistant(short int idEmp,QString ava,QString idCit,QString n,QString d,QString add,QString phone , QString gender,int baseSalary):Staff("Kitchen Assistant",idEmp,ava,idCit,n,d,add,phone,gender,baseSalary){}
};
#endif // KITCHENASSISTANT_H
