#include "global.h"
#ifndef PROFILE_VIEW_H
#define PROFILE_VIEW_H


#include "EditProfile_Widget.h"
#include "EditPassword_Widget.h"
#include "utils/SessionManage.h"
QT_BEGIN_NAMESPACE
namespace Ui { class Profile_View; }
QT_END_NAMESPACE

class EditProfile_Widget;

class Profile_View : public QWidget
{
    Q_OBJECT

private:
    void setupAvatar(const QString& imagePath);
    Ui::Profile_View* ui;

    EditProfile_Widget* editProfileWidget;
    EditPassword_Widget* editPasswordWidget;
    //Cur* currentUser;

public:
    explicit Profile_View(QWidget *parent = nullptr);
    ~Profile_View();

    void loadUserData();

    // Call this function to fill the UI with user data dynamically
    void loadUserData(const QString& name, const QString& studentId, const QString& dob,
                      const QString& phone, const QString& email, const QString& avatarPath);

protected:
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void showProfileUpdateResult(bool success, const QString& errorMsg = "");
    void showPasswordUpdateResult(bool success, const QString& errorMsg = "");

signals:
    void requestProfileUpdate(const QString& name, const QString& dob, const QString& address, const QString& phone, const QString& citizenId, const QString& avatarPath, const QString& gender);
    void requestPasswordUpdate(const QString& oldPassword, const QString& newPassword);
    void backToPrevious();

private slots:
    void on_backButton_clicked();
    void on_btnEditInfo_clicked();
    void on_btnEditPassword_clicked();
};

#endif // PROFILE_VIEW_H