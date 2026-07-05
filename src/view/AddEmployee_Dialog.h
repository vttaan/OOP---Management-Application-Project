#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

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
    // Auto-generated (not shown in UI): username = citizenId, password = default
    QString getUsername()   const;
    QString getPassword()   const;

private slots:
    void onConfirm();

private:
    void setupUi();
    bool validate();

    QString m_avatarPath;

    // Input fields
    QLineEdit *inpName;
    QLineEdit *inpPhone;
    QLineEdit *inpDob;
    QLineEdit *inpAddress;
    QLineEdit *inpCitizenId;
    QLineEdit *inpUsername;
    QLineEdit *inpPassword;
    QComboBox *cmbRole;
    QComboBox *cmbGender;

    // Avatar UI
    QLabel      *lblAvatarPreview;
    QPushButton *btnUpload;

    // Buttons
    QPushButton *btnConfirm;
    QPushButton *btnCancel;

    // Error label
    QLabel *lblError;
};
