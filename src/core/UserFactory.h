#pragma once
#include "core/Manager.h"
#include "core/Staff.h"
class UserFactory {
public:
<<<<<<< Updated upstream
    static User* createContainsUser(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add,
		QString phone);
    static User* createNewUser(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add,
        QString phone);
=======
  static User *createContainsUser(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add,
                                  QString phone, QString gender);
  static User *createNewUser(QString r, short int idEmp, QString ava, QString idCit, QString n, QString d, QString add,
                             QString phone, QString gender);
>>>>>>> Stashed changes
};

