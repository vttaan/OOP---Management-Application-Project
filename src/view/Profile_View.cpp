#include "Profile_View.h"
#include "ui_Profile_View.h"
#include <QPainter>
#include <QPainterPath>

Profile_View::Profile_View(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Profile_View)
{
    ui->setupUi(this);

    // Provide a default placeholder avatar if none is set yet
    setupAvatar(":/images/default_avatar.png");
}

Profile_View::~Profile_View()
{
    delete ui;
}

void Profile_View::setupAvatar(const QString& imagePath)
{
    QPixmap original(imagePath);
    if (original.isNull()) {
        return;
    }

    // Target avatar dimensions (180x180 px matching UI file limits)
    int size = 180;
    QPixmap scaled = original.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    // Apply soft rounded corners (12px border radius like the image)
    QPixmap rounded(size, size);
    rounded.fill(Qt::transparent);

    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath path;
    path.addRoundedRect(0, 0, size, size, 12, 12);
    painter.setClipPath(path);

    // Center and paint the avatar
    int xOffset = (size - scaled.width()) / 2;
    int yOffset = (size - scaled.height()) / 2;
    painter.drawPixmap(xOffset, yOffset, scaled);
    painter.end();

    ui->lblAvatar->setPixmap(rounded);
}

void Profile_View::loadUserData(const QString& name, const QString& studentId, const QString& dob,
                                const QString& phone, const QString& email, const QString& avatarPath)
{
    // Populate left info panel
    ui->lblProfileName->setText(name);
    setupAvatar(avatarPath);

    // Populate general details
    ui->lblVal_Id->setText(studentId);
    ui->lblVal_DoB->setText(dob);

    // Populate detailed details
    ui->lblVal_Phone->setText(phone);
    ui->lblVal_Email->setText(email);
}