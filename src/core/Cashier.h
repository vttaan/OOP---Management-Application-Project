#ifndef CASHIER_H
#define CASHIER_H
#include "global.h"
#include "core/Staff.h"
class Cashier:public Staff{
private:
    // Cashier co the la fulltime
    double fixedMonthlySalary=0;
    bool isFixed=false;
public:
  Cashier(short int idEmp, QString ava, QString idCit, QString n,
                    QString d, QString add, QString phone, QString gender, int baseSalary)
            : Staff("Cashier", idEmp, ava, idCit, n, d, add, phone, gender, baseSalary)
, isFixed(false) {}
Cashier(short int idEmp, QString ava, QString idCit, QString n, QString d, QString add, QString phone, QString gender, int baseSalary, double fixedSalary)
        : Staff("Cashier", idEmp, ava, idCit, n, d, add, phone, gender, baseSalary)
        , fixedMonthlySalary(fixedSalary), isFixed(true) {}

  double getSalary()const override{
      if(isFixed){
          return fixedMonthlySalary;
      }
      return Staff::getSalary();
  }
  double getBaseSalary() const override {
      if(isFixed) return fixedMonthlySalary;
      return Staff::getBaseSalary();
  }
  bool getIsFixedSalary() const override { return isFixed; }
};


#endif // CASHIER_H
