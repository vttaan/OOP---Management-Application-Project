#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "core/User.h"

class EditEmployee_Dialog : public QDialog {
    Q_OBJECT

public:
    // Nhận User* để pre-fill toàn bộ form
    explicit EditEmployee_Dialog(User *emp, QWidget *parent = nullptr);

    // Getters — Controller đọc sau khi dialog accepted
    QString getName()      const;
    QString getRole()      const;
    QString getPhone()     const;
    QString getDob()       const;
    QString getAddress()   const;
    QString getCitizenId() const;
    QString getAvatarPath() const;

private slots:
    void onConfirm();

private:
    void setupUi(User *emp);
    bool validate();

    QString m_avatarPath;

    QLineEdit *inpName;
    QLineEdit *inpPhone;
    QLineEdit *inpDob;
    QLineEdit *inpAddress;
    QLineEdit *inpCitizenId;
    QComboBox *cmbRole;

    QLabel *lblAvatarPreview;
    QPushButton *btnUpload;

    QPushButton *btnConfirm;
    QPushButton *btnCancel;
    QLabel      *lblError;
};
