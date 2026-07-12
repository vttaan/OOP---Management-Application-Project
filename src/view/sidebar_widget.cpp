#include "sidebar_widget.h"
#include "ui_sidebar_widget.h"
#include <QPainter>
#include <QStyleOption>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QDir>
#include <QCoreApplication>
#include "utils/SessionManage.h"
#include "core/User.h"

Sidebar_Widget::Sidebar_Widget(QWidget *parent) :
    QWidget(parent), ui(new Ui::Sidebar_Widget)
{
    ui->setupUi(this);
    this->setObjectName("Sidebar_Widget");
    initUI();
    ui->subMenu_Schedule->hide(); // setup in default, when start program

    // logic in Schedule tab, because Schedule tab has 3 subTab.
    connect(ui->buttonSchedule, &QPushButton::clicked, [this]() {
        bool isHidden = ui->subMenu_Schedule->isHidden();
        ui->subMenu_Schedule->setVisible(isHidden);
    });

    // main tab
    connect(ui->btnProfile, &QPushButton::clicked, [this]() { emit menuClicked(2); updateButtonStyles(2); });
    connect(ui->btnMenu_Overview, &QPushButton::clicked, [this]() { emit menuClicked(1); updateButtonStyles(1); });
    connect(ui->btnMenu_HR, &QPushButton::clicked, [this]() { emit menuClicked(3); updateButtonStyles(3); });
    //connect(ui->btnMenu_Salary, &QPushButton::clicked, [this]() { emit menuClicked(7); updateButtonStyles(7); });
    //connect(ui->btnMenu_Report, &QPushButton::clicked, [this]() { emit menuClicked(8); updateButtonStyles(8); });
    //connect(ui->btnMenu_Settings, &QPushButton::clicked, [this]() { emit menuClicked(9); updateButtonStyles(9); });

    // subTab in Schedule
    connect(ui->buttonRegistrationSchedule, &QPushButton::clicked, [this]() { emit menuClicked(4); updateButtonStyles(4); });
    connect(ui->buttonArrangeSchedule, &QPushButton::clicked, [this]() { emit menuClicked(5); updateButtonStyles(5); });
    connect(ui->buttonViewSchedule, &QPushButton::clicked, [this]() { emit menuClicked(6); updateButtonStyles(6); });

    connect(ui->btnLogout, &QPushButton::clicked, [this]() { emit logoutClicked(); });

    // default set view in dashboard
    updateButtonStyles(1);
}

Sidebar_Widget::~Sidebar_Widget() { delete ui; }

void Sidebar_Widget::initUI() {

    // setup background
    this->setStyleSheet("Sidebar_Widget { "
                        "   background-color: #f7f9fc; "
                        "   border-right: 1px solid #E5E7EB; "
                        "}");

    // setup logo
    if (ui->labelLogo) {
        ui->labelLogo->setText("Hệ thống quản lý\nnhân sự");
        ui->labelLogo->setAlignment(Qt::AlignCenter);
        ui->labelLogo->setStyleSheet(
            "font-size: 16px; "
            "font-weight: 900; "
            "color: #111827; "
            "border: none; "
            "margin-top: 10px; "
            "margin-bottom: 15px;"
            );
    }

    if (ui->btnProfile) {
        ui->btnProfile->setText("");
        ui->btnProfile->setStyleSheet(
            "QPushButton#btnProfile { "
            "   background-color: #ffffff; "
            "   border: 1px solid #e2e8f0; "
            "   border-radius: 12px; "
            "   margin: 5px 15px; "
            "   padding: 5px; "
            "   text-align: left; "
            "}"
            "QPushButton#btnProfile:hover { "
            "   background-color: #f8fafc; "
            "   border-color: #cbd5e1; "
            "}"
            "QPushButton#btnProfile:pressed { "
            "   background-color: #f1f5f9; "
            "}"
        );
        ui->btnProfile->setFixedHeight(70);
        QHBoxLayout* profileLayout = new QHBoxLayout(ui->btnProfile);
        profileLayout->setContentsMargins(20, 10, 12, 10);
        profileLayout->setSpacing(12);

        QLabel* lblAvatar = new QLabel(ui->btnProfile);
        lblAvatar->setObjectName("lblSidebarAvatar");
        lblAvatar->setFixedSize(36, 36);
        lblAvatar->setStyleSheet("background-color: #d2e3fc; border-radius: 18px; border: none;");
        lblAvatar->setAlignment(Qt::AlignCenter);
        lblAvatar->setText("👨‍💼");
        lblAvatar->setAttribute(Qt::WA_TransparentForMouseEvents, true);

        QVBoxLayout* textLayout = new QVBoxLayout();
        textLayout->setSpacing(2);
        textLayout->setContentsMargins(0, 0, 0, 0);

        QLabel* lblName = new QLabel("Quản trị viên", ui->btnProfile);
        lblName->setObjectName("lblSidebarName");
        lblName->setStyleSheet("font-size: 13px; font-weight: bold; color: #212b36; background: transparent; border: none;");
        lblName->setAttribute(Qt::WA_TransparentForMouseEvents, true);

        QLabel* lblRole = new QLabel("Quản trị hệ thống", ui->btnProfile);
        lblRole->setObjectName("lblSidebarRole");
        lblRole->setStyleSheet("font-size: 11px; color: #637381; background: transparent; border: none;");
        lblRole->setAttribute(Qt::WA_TransparentForMouseEvents, true);

        textLayout->addWidget(lblName);
        textLayout->addWidget(lblRole);

        profileLayout->addWidget(lblAvatar);
        profileLayout->addLayout(textLayout);
        profileLayout->addStretch();
    }

    // setup default
    updateButtonStyles(1);
}

void Sidebar_Widget::updateButtonStyles(int mainIndex)
{

    // CSS / AI GEN
    QString activeMain = this->getActiveStyle();
    QString activeSub = activeMain;
    activeSub.replace("padding-left: 35px;", "padding-left: 55px;");
    QString normal = this->getNormalStyle();
    QString normalSub = normal;
    normalSub.replace("padding-left: 35px;", "padding-left: 55px;");
    // Reset all
    ui->btnMenu_Overview->setStyleSheet(normal);
    ui->btnMenu_HR->setStyleSheet(normal);
    ui->buttonSchedule->setStyleSheet(normal);
    ui->btnMenu_Salary->setStyleSheet(normal);
    ui->btnMenu_Report->setStyleSheet(normal);
    ui->btnMenu_Settings->setStyleSheet(normal);
    ui->buttonRegistrationSchedule->setStyleSheet(normal);
    ui->buttonArrangeSchedule->setStyleSheet(normal);
    ui->buttonViewSchedule->setStyleSheet(normal);
    ui->btnLogout->setStyleSheet(normal);

    switch(mainIndex) {
    case 1: ui->btnMenu_Overview->setStyleSheet(activeMain); break;
    case 3: ui->btnMenu_HR->setStyleSheet(activeMain); break;
    case 4:
        ui->buttonSchedule->setStyleSheet(activeMain);
        ui->buttonRegistrationSchedule->setStyleSheet(activeSub);
        break;
    case 5:
        ui->buttonSchedule->setStyleSheet(activeMain);
        ui->buttonArrangeSchedule->setStyleSheet(activeSub);
        break;
    case 6:
        ui->buttonSchedule->setStyleSheet(activeMain);
        ui->buttonViewSchedule->setStyleSheet(activeSub);
        break;
    }
}

void Sidebar_Widget::loadUserData(SessionManager* session) {
    if (!session || !session->getCurrentUser()) return;
    User* user = session->getCurrentUser();

    QLabel* lblName = this->findChild<QLabel*>("lblSidebarName");
    QLabel* lblRole = this->findChild<QLabel*>("lblSidebarRole");

    if (lblName) lblName->setText(user->getName());
    if (lblRole) {
        QString roleText = user->getRole();
        if (roleText == "Manage") {
            lblRole->setText("Quản trị hệ thống");
        } else if (roleText == "Staff") {
            lblRole->setText("Nhân viên");
        } else {
            lblRole->setText(roleText);
        }
    }
    setupSidebarAvatar(user->getAvatarPath());
}

void Sidebar_Widget::setupSidebarAvatar(const QString& imagePath) {
    QLabel* lblAvatar = this->findChild<QLabel*>("lblSidebarAvatar");
    if (!lblAvatar) return;

    QDir appDir(QCoreApplication::applicationDirPath());
    appDir.cdUp(); // build
    appDir.cdUp(); // MAP
    QString folderPath = appDir.filePath("resources");

    QPixmap avatarPixmap;
    if (!imagePath.isEmpty() && QFile::exists(folderPath + "/avatars/" + imagePath)) {
        avatarPixmap.load(folderPath + "/avatars/" + imagePath);
    }

    if (avatarPixmap.isNull()) {
        avatarPixmap.load(":/images/avatarSample.png");
    }

    int size = 36;
    QPixmap scaled = avatarPixmap.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    QPixmap rounded(size, size);
    rounded.fill(Qt::transparent);

    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath path;
    path.addRoundedRect(0, 0, size, size, size / 2, size / 2);
    painter.setClipPath(path);

    int xOffset = (size - scaled.width()) / 2;
    int yOffset = (size - scaled.height()) / 2;
    painter.drawPixmap(xOffset, yOffset, scaled);
    painter.end();

    lblAvatar->setPixmap(rounded);
}

void Sidebar_Widget::paintEvent(QPaintEvent *event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}