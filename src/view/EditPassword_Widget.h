#ifndef EDITPASSWORD_WIDGET_H
#define EDITPASSWORD_WIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QPainter>

// Class inherits from Qlineedit specifies for password w/ functions like toggle hide/show, setup
class password_LineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit password_LineEdit(QWidget* parent = nullptr) : QLineEdit(parent) {
        toggleIcon = this->addAction(
            QIcon(":/images/eyeOpen.svg"),
            QLineEdit::TrailingPosition
        );
        connect(toggleIcon, &QAction::triggered, this, &password_LineEdit::togglePasword);
        this->setEchoMode(QLineEdit::Password);
    }
    
    QAction* toggleIcon;
    void setup() {
        this->setText("");
        this->setEchoMode(QLineEdit::Password);
    }
    void togglePasword() {
        if (this->echoMode() == QLineEdit::Password) {
            this->setEchoMode(QLineEdit::Normal);
            toggleIcon->setIcon(QIcon(":/images/eyeClosed.svg"));
        } else {
            this->setEchoMode(QLineEdit::Password);
            toggleIcon->setIcon(QIcon(":/images/eyeOpen.svg"));
        }
    }
};

class EditPassword_Widget : public QWidget
{
    Q_OBJECT
public:
    explicit EditPassword_Widget(QWidget *parent = nullptr);
    void setInitialData();
    void slideIn();
    void slideOut();
    password_LineEdit* txtOldPassword;
    password_LineEdit* txtNewPassword;
    password_LineEdit* txtConfirmPassword;
signals:
    void saveRequested(const QString& password);
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onAnimationFinished();
private:
    QWidget *panelWidget;
    QPropertyAnimation *animation;


    QLabel *lblAvatarPreview;
    QString currentAvatarPath;

    bool isPanelOpen;
};

#endif // EDITPASSWORD_WIDGET_H
