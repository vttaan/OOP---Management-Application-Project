#ifndef PROFILE_VIEW_H
#define PROFILE_VIEW_H

#include <QWidget>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>

QT_BEGIN_NAMESPACE
namespace Ui { class Profile_View; }
QT_END_NAMESPACE

class Profile_View : public QWidget
{
    Q_OBJECT

private:
    Ui::Profile_View *ui;
    void setupAvatar(const QString& imagePath);

public:
    explicit Profile_View(QWidget *parent = nullptr);
    ~Profile_View();

    // Call this function to fill the UI with user data dynamically
    void loadUserData(const QString& name, const QString& studentId, const QString& dob,
                      const QString& phone, const QString& email, const QString& avatarPath);

signals:
    void backToPrevious();
private slots:
    void on_backButton_clicked();
};

#endif // PROFILE_VIEW_H