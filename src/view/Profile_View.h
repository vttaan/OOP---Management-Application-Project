#ifndef PROFILE_VIEW_H
#define PROFILE_VIEW_H

#include <QWidget>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include "control/Profile_Control.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Profile_View; }
QT_END_NAMESPACE

class Profile_View : public QWidget
{
    Q_OBJECT

private:
    void setupAvatar(const QString& imagePath);
    Profile_Control* controller;
    Ui::Profile_View* ui;

public:
    explicit Profile_View(Profile_Control* controller = nullptr, QWidget *parent = nullptr);
    ~Profile_View();
    Profile_Control* getController();

    // Call this function to fill the UI with user data dynamically
    void loadUserData(const QString& name, const QString& studentId, const QString& dob,
                      const QString& phone, const QString& email, const QString& avatarPath);

signals:
private slots:
    void on_backButton_clicked();
};

#endif // PROFILE_VIEW_H