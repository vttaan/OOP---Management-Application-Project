#pragma once

#include "global.h"
class User {
protected:
	QString role;
	short int idEmployee;
	QString avatarPath;
	QString idCitizenIdentity;
	QString name;
	QString dob;
	QString address;
	QString phoneNum;
    QString gender;
public:
    User(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add, QString phone ,QString gender);
	virtual ~User() = default;
    virtual double getSalary() const = 0;
    virtual double getBaseSalary() const = 0;

    virtual bool getIsFixedSalary() const { return false; }

    virtual User* clone() const = 0;


    virtual void setBaseSalary(double salary) = 0;


    // child class of Staff implement
    virtual void setFixedEmployee(bool isFixed) {}
    virtual void setAllowence() {}
    virtual void setAllowenceValue(double allowance) {}

    virtual bool getIsFixedEmployee() const { return false; }

	QString getRole() const;
	short int getIdEmployee() const;
    QString getIdentityID() const;
	QString getName() const;
	QString getDOB() const;
	QString getAddress() const;
	QString getAvatarPath() const;
	QString getPhoneNum() const;
    QString getGender() const;
    QString getAnyAttributes(QString content) const;
	void setRole(QString r);
    void setGender(QString g);
	void setIdEmployee(short int id);
	void setAva(QString a);
	void setIndentityID(QString idCit);
	void setName(QString n);
	void setAddress(QString add);
	void setDOB(QString d);
	void setPhoneNum(QString phone);
};