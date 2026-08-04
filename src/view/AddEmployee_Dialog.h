#pragma once

#include "global.h"
#include <functional>
#include <QPair>

class AddEmployee_Dialog : public QDialog {
    Q_OBJECT

public:
    explicit AddEmployee_Dialog(QWidget *parent = nullptr);

    // Getters — Controller reads after dialog is accepted
    QString getName()       const;
    QString getRole()       const;
    QString getGender()     const;
    QString getPhone()      const;
    QString getDob()        const;
    QString getAddress()    const;
    QString getCitizenId()  const;
    QString getAvatarPath() const;
    int getSalary()     const;
    bool getIsFixedSalary() const;
    bool isDobSelected() const;
    // Auto-generated (not shown in UI): username = citizenId, password = default
    QString getUsername()   const;
    QString getPassword()   const;
    std::function<QString(AddEmployee_Dialog*)> validatorDelegate;
    std::function<QString(const QString& name, const QString& dob)> passwordGeneratorDelegate;
    std::function<QString(const QString& role)> usernameGeneratorDelegate;
private slots:
    void onConfirm();

private:
    void setupUi();
    bool validate();

    QString m_avatarPath;

    // Input fields
    QLineEdit *inpName;
    QLineEdit *inpPhone;
    QDateEdit *inpDob;
    QLineEdit *inpAddress;
    QLineEdit *inpCitizenId;
    QLineEdit *inpUsername;
    QLineEdit *inpPassword;
    QLineEdit *inpSalary;
    QComboBox *cmbRole;
    QComboBox *cmbGender;
    QComboBox *cmbIsFixedSalary;

    // Avatar UI
    QLabel      *lblAvatarPreview;
    QPushButton *btnUpload;

    // Buttons
    QPushButton *btnConfirm;
    QPushButton *btnCancel;

    // Error label
    QLabel *lblError;

};
