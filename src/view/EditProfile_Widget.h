#pragma once

#include "global.h"
class EditProfile_Widget : public QWidget {
    Q_OBJECT

public:
    explicit EditProfile_Widget(QWidget *parent = nullptr);
    void setInitialData(const QString& name, const QString& dob, const QString& address,
                        const QString& phone, const QString& citizenId, const QString& avatarPath, const QString& gender);
    void slideIn();
    void slideOut();

signals:
    void saveRequested(const QString& name, const QString& dob, const QString& address,
                       const QString& phone, const QString& citizenId, const QString& avatarPath, const QString& gender);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onChangeAvatarClicked();
    void onAnimationFinished();

private:
    QWidget *panelWidget;
    QPropertyAnimation *animation;
    
    QLineEdit *txtName;
    QLineEdit *txtDob;
    QLineEdit *txtAddress;
    QLineEdit *txtPhone;
    QLineEdit *txtCitizenId;
    QComboBox *cmbGender;
    QLabel *lblAvatarPreview;
    QString currentAvatarPath;
    
    bool isPanelOpen;
};
