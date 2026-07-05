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

    // Getters — Controller đọc sau khi dialog accepted
    QString getName()       const;
    QString getRole()       const;
    QString getPhone()      const;
    QString getDob()        const;
    QString getAddress()    const;
    QString getCitizenId()  const;
    QString getUsername()   const;
    QString getPassword()   const;
    QString getAvatarPath() const;

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

    // --- Avatar UI ---
    QLabel *lblAvatarPreview;
    QPushButton *btnUpload;
    // Buttons
    QPushButton *btnConfirm;
    QPushButton *btnCancel;

    // Error label
    QLabel *lblError;
};
