#ifndef VALIDATOR_H
#define VALIDATOR_H

class Validator
{
public:
    static bool isValidPassword(const QString& password);
    static bool isValidDate(const QString& datestring);
    static bool isValidPhoneNumber(const QString& num);
    static bool isValidCitizenId(const QString& citizenId);
};

#endif // VALIDATOR_H
