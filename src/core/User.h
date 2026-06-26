#pragma once
#include <QString>


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
public:
	User(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add, QString phone);
	virtual ~User() = default;
	virtual double getSalary() const = 0;
	QString getRole() const;
	short int getIdEmployee() const;
	QString getIndentityID() const;
	QString getName() const;
	QString getDOB() const;
	QString getAddress() const;
	QString getAvatarPath() const;
	QString getPhoneNum() const;
	void setRole(QString r);
	void setIdEmployee(short int id);
	void setAva(QString a);
	void setIndentityID(QString idCit);
	void setName(QString n);
	void setAddress(QString add);
	void setDOB(QString d);
	void setPhoneNum(QString phone);
};