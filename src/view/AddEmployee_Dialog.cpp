#include "AddEmployee_Dialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QFile>

AddEmployee_Dialog::AddEmployee_Dialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Thêm nhân viên mới");
    setMinimumWidth(420);
    setModal(true);
    m_avatarPath = "";
    setupUi();
}


// ============================================================
// UI Setup
// ============================================================
void AddEmployee_Dialog::setupUi()
{
    this->setObjectName("AddEmployee_Dialog");

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(28, 24, 28, 24);

    // Title
    QLabel *lblTitle = new QLabel("Thêm nhân viên mới");
    lblTitle->setObjectName("dlgTitle");
    mainLayout->addWidget(lblTitle);

    QLabel *lblSub = new QLabel("Điền đầy đủ thông tin bên dưới để tạo nhân viên.");
    lblSub->setObjectName("dlgSub");
    mainLayout->addWidget(lblSub);

    // Divider
    QFrame *divider = new QFrame();
    divider->setObjectName("dlgDivider");
    divider->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(divider);

    // Form
    QFormLayout *form = new QFormLayout();
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    auto makeInput = [](const QString &placeholder) -> QLineEdit* {
        QLineEdit *inp = new QLineEdit();
        inp->setPlaceholderText(placeholder);
        inp->setMinimumHeight(32);
        return inp;
    };

    auto makeLabel = [](const QString &text) -> QLabel* {
        QLabel *lbl = new QLabel(text);
        lbl->setObjectName("formLabel");
        return lbl;
    };

    inpName      = makeInput("vd: Nguyễn Văn A");
    inpPhone     = makeInput("vd: 0901234567");
    inpDob       = makeInput("YYYY-MM-DD");
    inpAddress   = makeInput("vd: 123 Lê Lợi, TP.HCM");
    inpCitizenId = makeInput("vd: 012345678901");

    cmbRole = new QComboBox();
    cmbRole->addItem("Nhân viên");
    cmbRole->addItem("Quản lý");
    cmbRole->setMinimumHeight(32);

    cmbGender = new QComboBox();
    cmbGender->addItem("Nam");
    cmbGender->addItem("Nữ");
    cmbGender->addItem("Khác");
    cmbGender->setMinimumHeight(32);

    // Avatar upload section
    lblAvatarPreview = new QLabel("Chưa có ảnh");
    lblAvatarPreview->setObjectName("lblAvatarPreview");
    lblAvatarPreview->setFixedSize(100, 100);
    lblAvatarPreview->setAlignment(Qt::AlignCenter);

    btnUpload = new QPushButton("Chọn ảnh...");
    btnUpload->setObjectName("btnUpload");
    btnUpload->setCursor(Qt::PointingHandCursor);

    QHBoxLayout *avatarLayout = new QHBoxLayout();
    avatarLayout->addWidget(lblAvatarPreview);
    avatarLayout->addWidget(btnUpload);
    avatarLayout->addStretch();

    // Connect upload button
    connect(btnUpload, &QPushButton::clicked, this, [=]() {
        QString filePath = QFileDialog::getOpenFileName(this,
                                                        "Chọn ảnh đại diện",
                                                        "",
                                                        "Hình ảnh (*.png *.jpg *.jpeg)");
        if (!filePath.isEmpty()) {
            m_avatarPath = filePath;
            QPixmap pix(filePath);
            if (!pix.isNull()) {
                int size = 100;
                QPixmap target(size, size);
                target.fill(Qt::transparent);

                QPixmap scaledSrc = pix.scaled(size, size,
                                               Qt::KeepAspectRatioByExpanding,
                                               Qt::SmoothTransformation);

                QPainter painter(&target);
                painter.setRenderHint(QPainter::Antialiasing, true);
                painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

                QPainterPath path;
                path.addEllipse(0, 0, size, size);
                painter.setClipPath(path);

                int x = (scaledSrc.width()  - size) / 2;
                int y = (scaledSrc.height() - size) / 2;
                painter.drawPixmap(0, 0, scaledSrc, x, y, size, size);

                lblAvatarPreview->setPixmap(target);
            }
        }
    });

    // Add rows to form
    form->addRow(makeLabel("Ảnh đại diện"),    avatarLayout);
    form->addRow(makeLabel("Họ và tên *"),      inpName);
    form->addRow(makeLabel("Vai trò *"),        cmbRole);
    form->addRow(makeLabel("Giới tính"),        cmbGender);
    form->addRow(makeLabel("Số điện thoại"),    inpPhone);
    form->addRow(makeLabel("Ngày sinh"),        inpDob);
    form->addRow(makeLabel("Địa chỉ"),          inpAddress);
    form->addRow(makeLabel("CCCD / CMND *"),    inpCitizenId);

    // Account credentials section
    QLabel *lblAccount = new QLabel("— Thông tin tài khoản —");
    lblAccount->setObjectName("lblAccount");
    lblAccount->setAlignment(Qt::AlignCenter);
    form->addRow(lblAccount);

    inpUsername = makeInput("Tên đăng nhập");
    inpPassword = makeInput("Mật khẩu");
    inpPassword->setEchoMode(QLineEdit::Password);

    form->addRow(makeLabel("Tên đăng nhập *"), inpUsername);
    form->addRow(makeLabel("Mật khẩu *"),      inpPassword);

    mainLayout->addLayout(form);

    // Error label
    lblError = new QLabel();
    lblError->setObjectName("lblError");
    lblError->setWordWrap(true);
    lblError->setVisible(false);
    mainLayout->addWidget(lblError);

    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    btnCancel = new QPushButton("Hủy");
    btnCancel->setObjectName("btnCancel");
    btnCancel->setMinimumHeight(36);
    btnCancel->setCursor(Qt::PointingHandCursor);

    btnConfirm = new QPushButton("Thêm nhân viên");
    btnConfirm->setObjectName("btnConfirm");
    btnConfirm->setMinimumHeight(36);
    btnConfirm->setCursor(Qt::PointingHandCursor);

    btnLayout->addStretch();
    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnConfirm);
    mainLayout->addLayout(btnLayout);

    // Connections
    connect(btnConfirm, &QPushButton::clicked, this, &AddEmployee_Dialog::onConfirm);
    connect(btnCancel,  &QPushButton::clicked, this, &QDialog::reject);

    QFile file(":/styles/Employee_View_styles.qss");
    if (file.open(QFile::ReadOnly)) {
        this->setStyleSheet(QLatin1String(file.readAll()));
        file.close();
    }
}

// ============================================================
// Validate + Accept
// ============================================================
bool AddEmployee_Dialog::validate()
{
    if (inpName->text().trimmed().isEmpty()) {
        lblError->setText("⚠  Họ và tên là bắt buộc.");
        lblError->setVisible(true);
        inpName->setFocus();
        return false;
    }
    if (inpCitizenId->text().trimmed().isEmpty()) {
        lblError->setText("⚠  Số CCCD / CMND là bắt buộc.");
        lblError->setVisible(true);
        inpCitizenId->setFocus();
        return false;
    }
    if (inpUsername->text().trimmed().isEmpty()) {
        lblError->setText("⚠  Tên đăng nhập là bắt buộc.");
        lblError->setVisible(true);
        inpUsername->setFocus();
        return false;
    }
    if (inpPassword->text().isEmpty()) {
        lblError->setText("⚠  Mật khẩu là bắt buộc.");
        lblError->setVisible(true);
        inpPassword->setFocus();
        return false;
    }
    lblError->setVisible(false);
    return true;
}

void AddEmployee_Dialog::onConfirm()
{
    if (validate())
        accept();
}

// ============================================================
// Getters
// ============================================================

QString AddEmployee_Dialog::getName()       const { return inpName->text().trimmed(); }

// Role is stored in English internally; map from Vietnamese display text
QString AddEmployee_Dialog::getRole()       const
{
    QString vn = cmbRole->currentText();
    if (vn == "Quản lý") return "Manager";
    return "Staff";
}

QString AddEmployee_Dialog::getGender()     const { return cmbGender->currentText(); }
QString AddEmployee_Dialog::getPhone()      const { return inpPhone->text().trimmed(); }
QString AddEmployee_Dialog::getDob()        const { return inpDob->text().trimmed(); }
QString AddEmployee_Dialog::getAddress()    const { return inpAddress->text().trimmed(); }
QString AddEmployee_Dialog::getCitizenId()  const { return inpCitizenId->text().trimmed(); }
QString AddEmployee_Dialog::getAvatarPath() const { return m_avatarPath; }

// Auto-generated credentials — username and password from real input fields
QString AddEmployee_Dialog::getUsername()   const { return inpUsername->text().trimmed(); }
QString AddEmployee_Dialog::getPassword()   const { return inpPassword->text(); }