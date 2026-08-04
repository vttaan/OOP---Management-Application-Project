#ifndef EMPLOYEEDETAILS_DIALOG_H
#define EMPLOYEEDETAILS_DIALOG_H

#include "global.h"
#include "core/User.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QFormLayout>
#include <QFrame>

class EmployeeDetails_Dialog : public QDialog {
    Q_OBJECT
public:
    explicit EmployeeDetails_Dialog(User* emp, QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QPoint dragPosition;
    void setupUi(User* emp);
    QString getVietnameseRole(const QString& roleName);
    QPixmap getRoundedAvatar(const QString& avatarPath, const QString& name, const QString& role, int size);
    void addRowToForm(QFormLayout* form, const QString& labelText, const QString& valueText, bool isHighlight = false);
};

#endif // EMPLOYEEDETAILS_DIALOG_H
