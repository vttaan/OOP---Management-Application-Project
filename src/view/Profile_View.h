#ifndef PROFILE_VIEW_H
#define PROFILE_VIEW_H

#include <QMainWindow>

namespace Ui {
class Profile_View;
}

QT_BEGIN_NAMESPACE
namespace Ui { class Profile_View; }
QT_END_NAMESPACE

class Profile_View : public QMainWindow
{
    Q_OBJECT

public:
    explicit Profile_View(QWidget *parent = nullptr);
    ~Profile_View();
    void loadUserData(const QString& name, const QString& studentId, const QString& dob,
                      const QString& phone, const QString& email, const QString& avatarPath);
private:
    Ui::Profile_View *ui;
    void setupAvatar(const QString& imagePath);
};

#endif // PROFILE_VIEW_H
