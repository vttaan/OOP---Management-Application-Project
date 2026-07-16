#include "Validator.h"
#include<QRegularExpression>
#include<QDate>


bool Validator::isValidPassword(const QString& password){
    QRegularExpression regex("^(?=.*[A-Z])(?=.*[0-9])(?=.*[@#&%].{6,}$");
    return regex.match(password).hasMatch();
}

bool Validator::isValidDate(const QString& datestring){
    return QDate::fromString(datestring, "yyyy-MM-dd").isValid();
}

bool Validator::isValidCitizenId(const QString &citizenId){
    QRegularExpression regex("^\\d{12}$");
    return regex.match(citizenId).hasMatch();
}

bool Validator::isValidPhoneNumber(const QString &num){
    QRegularExpression regex("^\\d{10}$");
    return regex.match(num).hasMatch();
}
