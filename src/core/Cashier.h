#ifndef CASHIER_H
#define CASHIER_H
#include "global.h"
#include "core/Staff.h"
class Cashier:public Staff{
private:
    double allowanceCashier;
public:
  Cashier(short int idEmp, QString ava, QString idCit, QString n,
                    QString d, QString add, QString phone, QString gender, double baseSalary, bool isFixedEmployee, double allowence)
        : Staff("Cashier", idEmp, ava, idCit, n, d, add, phone, gender, baseSalary, isFixedEmployee), allowanceCashier(allowence) {}

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

  double getAllowence() const override;
  void setAllowence() override;
  void setAllowenceValue(double allowance) override { allowanceCashier = allowance; }
  User* clone() const override { return new Cashier(*this); }

};

#endif // CASHIER_H
