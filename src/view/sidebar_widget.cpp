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

Sidebar_Widget::Sidebar_Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Sidebar_Widget)
{
    ui->setupUi(this);
    this->setObjectName("Sidebar_Widget");
    initUI();
    ui->subMenu_Schedule->hide(); // setup in default, when start program

    // logic in Schedule tab, because Schedule tab has 3 subTab.
    connect(ui->buttonSchedule, &QPushButton::clicked, [this]()
            {
        bool isHidden = ui->subMenu_Schedule->isHidden();
        ui->subMenu_Schedule->setVisible(isHidden); });

    // main tab
    connect(ui->btnProfile, &QPushButton::clicked, [this]()
            { emit menuClicked(2); updateButtonStyles(2); });
    connect(ui->btnMenu_Overview, &QPushButton::clicked, [this]()
            { emit menuClicked(1); updateButtonStyles(1); });
    connect(ui->btnMenu_HR, &QPushButton::clicked, [this]()
            { emit menuClicked(3); updateButtonStyles(3); });

    // subTab in Schedule
    connect(ui->buttonRegistrationSchedule, &QPushButton::clicked, [this]()
            { emit menuClicked(4); updateButtonStyles(4); });
    connect(ui->buttonArrangeSchedule, &QPushButton::clicked, [this]()
            { emit menuClicked(5); updateButtonStyles(5); });
    connect(ui->buttonViewSchedule, &QPushButton::clicked, [this]()
            { emit menuClicked(6); updateButtonStyles(6); });
    connect(ui->btnMenu_Salary, &QPushButton::clicked, [this]()
            { emit menuClicked(7); updateButtonStyles(7); });
    // connect(ui->btnMenu_Report, &QPushButton::clicked, [this]() { emit menuClicked(8); updateButtonStyles(8); });
    // connect(ui->btnMenu_Settings, &QPushButton::clicked, [this]() { emit menuClicked(9); updateButtonStyles(9); });

    connect(ui->btnLogout, &QPushButton::clicked, [this]()
            { emit logoutClicked(); });

    // default set view in dashboard
    updateButtonStyles(1);

    // permission default
    setPermission(false);
}

void Sidebar_Widget::setPermission(const bool &permitted)
{
    ui->btnMenu_HR->setVisible(permitted);
    ui->buttonArrangeSchedule->setVisible(permitted);
    ui->buttonRegistrationSchedule->setVisible(!permitted);
    ui->subMenu_Schedule->hide();
}

Sidebar_Widget::~Sidebar_Widget() { delete ui; }

void Sidebar_Widget::initUI()
{

    // Cyan gradient sidebar background
    this->setStyleSheet("Sidebar_Widget { "
                        "   background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0284C7, stop:1 #06B6D4); "
                        "   border-right: 1px solid rgba(255, 255, 255, 0.2); "
                        "}");

    // Logo
    if (ui->labelLogo)
    {
        ui->labelLogo->setText("Hệ thống quản lý\nnhân sự");
        ui->labelLogo->setAlignment(Qt::AlignCenter);
        ui->labelLogo->setStyleSheet(
            "font-size: 15px; "
            "font-weight: 900; "
            "color: #FFFFFF; "
            "border: none; "
            "margin-top: 12px; "
            "margin-bottom: 14px;");
    }

    if (ui->btnProfile)
    {
        ui->btnProfile->setText("");
        ui->btnProfile->setStyleSheet(
            "QPushButton#btnProfile { "
            "   background-color: rgba(255, 255, 255, 0.15); "
            "   border: 1px solid rgba(255, 255, 255, 0.3); "
            "   border-radius: 12px; "
            "   margin: 5px 12px; "
            "   padding: 8px; "
            "   text-align: left; "
            "}"
            "QPushButton#btnProfile:hover { "
            "   background-color: rgba(255, 255, 255, 0.25); "
            "   border-color: rgba(255, 255, 255, 0.4); "
            "}"
            "QPushButton#btnProfile:pressed { "
            "   background-color: rgba(255, 255, 255, 0.1); "
            "}");
        ui->btnProfile->setFixedHeight(82);
        QHBoxLayout *profileLayout = new QHBoxLayout(ui->btnProfile);
        profileLayout->setContentsMargins(16, 0, 12, 0); // Shift avatar off the left border a bit
        profileLayout->setSpacing(12);
        profileLayout->setAlignment(Qt::AlignVCenter);

        // --- Avatar (44x44 circle) ---
        QLabel *lblAvatar = new QLabel(ui->btnProfile);
        lblAvatar->setObjectName("lblSidebarAvatar");
        lblAvatar->setFixedSize(44, 44);
        lblAvatar->setStyleSheet("background-color: #3B82F6; border-radius: 22px; border: 2px solid #93C5FD;");
        lblAvatar->setAlignment(Qt::AlignCenter);
        lblAvatar->setText("👤");
        lblAvatar->setAttribute(Qt::WA_TransparentForMouseEvents, true);

        // --- Text column: Name + Role pill ---
        QVBoxLayout *textLayout = new QVBoxLayout();
        textLayout->setSpacing(5);
        textLayout->setContentsMargins(0, 0, 0, 0);

        QLabel *lblName = new QLabel("Quản trị viên", ui->btnProfile);
        lblName->setObjectName("lblSidebarName");
        lblName->setStyleSheet(
            "font-size: 15px; font-weight: bold; color: #FFFFFF; "
            "background: transparent; border: none;");
        lblName->setAttribute(Qt::WA_TransparentForMouseEvents, true);

        // Role pill badge (default: Admin style)
        QLabel *lblRole = new QLabel("Quản trị viên", ui->btnProfile);
        lblRole->setObjectName("lblSidebarRole");
        lblRole->setAlignment(Qt::AlignCenter);
        lblRole->setFixedHeight(20);
        lblRole->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        lblRole->setStyleSheet(
            "font-size: 11px; font-weight: 800; "
            "color: #1D4ED8; background-color: #DBEAFE; "
            "border-radius: 10px; padding: 0px 8px; border: none;");
        // Clamp width to content + padding
        lblRole->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        lblRole->adjustSize();

        textLayout->addWidget(lblName);
        textLayout->addWidget(lblRole, 0, Qt::AlignLeft);

        profileLayout->addWidget(lblAvatar);
        profileLayout->addLayout(textLayout);
        profileLayout->addStretch(); // trailing stretch pushes content left
    }

    // ---- Add SVG icons to all menu buttons ----
    auto applyIcon = [](QPushButton *btn, const QString &path, int sz = 24)
    {
        if (!btn)
            return;
        btn->setIcon(QIcon(path));
        btn->setIconSize(QSize(sz, sz));
    };
    applyIcon(ui->btnMenu_Overview, ":/images/dashboard-light.svg");
    applyIcon(ui->btnMenu_HR, ":/images/employee-light.svg");
    applyIcon(ui->buttonSchedule, ":/images/calendar-light.svg");
    applyIcon(ui->btnMenu_Salary, ":/images/report-white.svg");
    applyIcon(ui->btnMenu_Settings, ":/images/setting-light.svg");
    applyIcon(ui->btnLogout, ":/images/exit-light.svg");

    // Sub-menu frame — glassmorphic background
    if (ui->subMenu_Schedule)
    {
        ui->subMenu_Schedule->setStyleSheet("background-color: rgba(255, 255, 255, 0.1); border-radius: 6px;");
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
    ui->btnMenu_Settings->setStyleSheet(normal);
    ui->buttonRegistrationSchedule->setStyleSheet(normal);
    ui->buttonArrangeSchedule->setStyleSheet(normal);
    ui->buttonViewSchedule->setStyleSheet(normal);
    ui->btnLogout->setStyleSheet(logOutStyle);

    switch (mainIndex)
    {
    case 1:
        ui->btnMenu_Overview->setStyleSheet(activeMain);
        break;
    case 3:
        ui->btnMenu_HR->setStyleSheet(activeMain);
        break;
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
    case 7:
        ui->btnMenu_Salary->setStyleSheet(activeMain);
        break;
    }
}

void Sidebar_Widget::loadUserData(SessionManager *session)
{
    if (!session || !session->getCurrentUser())
        return;
    User *user = session->getCurrentUser();

    QLabel *lblName = this->findChild<QLabel *>("lblSidebarName");
    QLabel *lblRole = this->findChild<QLabel *>("lblSidebarRole");

    if (lblName)
        lblName->setText(user->getName());

    if (lblRole)
    {
        QString roleInternal = user->getRole();
        QString displayText;
        QString pillStyle;

        if (roleInternal == "Manager")
        {
            displayText = "Quản lý";
            // Purple pill — matches employee table Manager badge
            pillStyle = "font-size: 11px; font-weight: 800; "
                        "color: #6D28D9; background-color: #EDE9FE; "
                        "border-radius: 10px; padding: 0px 8px; border: none;";
        }
        else if (roleInternal == "Admin")
        {
            displayText = "Quản trị viên";
            // Blue pill
            pillStyle = "font-size: 11px; font-weight: 800; "
                        "color: #1D4ED8; background-color: #DBEAFE; "
                        "border-radius: 10px; padding: 0px 8px; border: none;";
        }
        else
        {
            displayText = "Nhân viên";
            // Light blue pill — matches employee table Staff badge
            pillStyle = "font-size: 11px; font-weight: 800; "
                        "color: #0369A1; background-color: #E0F2FE; "
                        "border-radius: 10px; padding: 0px 8px; border: none;";
        }

        lblRole->setText(displayText);
        lblRole->setStyleSheet(pillStyle);
        lblRole->adjustSize();
    }
    setupSidebarAvatar(user->getAvatarPath());
}

void Sidebar_Widget::setupSidebarAvatar(const QString &imagePath)
{
    QLabel *lblAvatar = this->findChild<QLabel *>("lblSidebarAvatar");
    if (!lblAvatar)
        return;

    QDir appDir(QCoreApplication::applicationDirPath());
    appDir.cdUp(); // build
    appDir.cdUp(); // MAP
    QString folderPath = appDir.filePath("resources");

    QPixmap avatarPixmap;
    if (!imagePath.isEmpty() && QFile::exists(folderPath + "/avatars/" + imagePath))
    {
        avatarPixmap.load(folderPath + "/avatars/" + imagePath);
    }

    if (avatarPixmap.isNull())
    {
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

void Sidebar_Widget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}