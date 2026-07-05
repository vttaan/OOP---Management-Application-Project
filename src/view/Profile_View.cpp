#include "Profile_View.h"
#include "ui_Profile_View.h"
#include <QPainter>
#include <QPainterPath>
#include <QFileInfo>
#include <QDir>

Profile_View::Profile_View(Profile_Control* controller, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Profile_View)
    , controller(controller)
{
    ui->setupUi(this);
    ui->backButton->setIcon(QIcon(":/images/homeIcon.png"));
    // Provide a default placeholder avatar if none is set yet
    setupAvatar(""); // default avt
}

Profile_View::~Profile_View()
{
    delete ui;
}

void Profile_View::setupAvatar(const QString& imagePath)
{
    QDir appDir(QCoreApplication::applicationDirPath()); // ......./Debug
    appDir.cdUp(); // ..../MAP/build
    appDir.cdUp(); // ..../MAP
    QString folderPath = appDir.filePath("resources"); // .../MAP/resources
    qDebug() << folderPath;

    QPixmap avatarPixmap(folderPath + "/" + imagePath);
    if(!imagePath.isEmpty() && QFileInfo::exists(imagePath)) { // image exists
        qDebug() << "found";
        avatarPixmap.load(imagePath);
    }

    if(avatarPixmap.isNull()) {
        qDebug() << "notfound";
        avatarPixmap.load(":/images/avatarSample.png");
    }

    // Target avatar dimensions (180x180 px matching UI file limits)
    int size = 180;
    QPixmap scaled = avatarPixmap.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);


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

Profile_Control* Profile_View::getController() {
    return controller;
}

void Profile_View::on_backButton_clicked()
{
    emit this->getController()->backToPrevious();
}

