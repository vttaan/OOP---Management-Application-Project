#include "AddEmployee_Dialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>

AddEmployee_Dialog::AddEmployee_Dialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Add New Employee");
    setMinimumWidth(420);
    setModal(true);


    m_avatarPath = "";

    setupUi();
}


// UI Setup
void AddEmployee_Dialog::setupUi()
{
    this->setObjectName("AddEmployee_Dialog");
    //Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(28, 24, 28, 24);

    //Title
    QLabel *lblTitle = new QLabel("Add New Employee");
    lblTitle->setObjectName("dlgTitle");
    mainLayout->addWidget(lblTitle);

    QLabel *lblSub = new QLabel("Fill in all fields below to create a new employee.");
    lblSub->setObjectName("dlgSub");
    mainLayout->addWidget(lblSub);

    // --- Divider ---
    QFrame *divider = new QFrame();
    divider->setObjectName("dlgDivider");
    divider->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(divider);

    // --- Form ---
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

    inpName      = makeInput("e.g. Nguyen Van A");
    inpPhone     = makeInput("e.g. 0901234567");
    inpDob       = makeInput("YYYY-MM-DD");
    inpAddress   = makeInput("e.g. 123 Le Loi, Ho Chi Minh");
    inpCitizenId = makeInput("e.g. 012345678901");
    inpUsername  = makeInput("Login username");
    inpPassword  = makeInput("Login password");
    inpPassword->setEchoMode(QLineEdit::Password);

    cmbRole = new QComboBox();
    cmbRole->addItem("Staff");
    cmbRole->addItem("Manager");
    cmbRole->setMinimumHeight(32);

    //AVATAR UPLOAD SECTION
    lblAvatarPreview = new QLabel("No Avatar");
    lblAvatarPreview->setObjectName("lblAvatarPreview");
    lblAvatarPreview->setFixedSize(100, 100);
    lblAvatarPreview->setAlignment(Qt::AlignCenter);

    btnUpload = new QPushButton("Browse...");
    btnUpload->setObjectName("btnUpload");
    btnUpload->setCursor(Qt::PointingHandCursor);

    QHBoxLayout *avatarLayout = new QHBoxLayout();
    avatarLayout->addWidget(lblAvatarPreview);
    avatarLayout->addWidget(btnUpload);
    avatarLayout->addStretch();

    //Connect Upload button
    connect(btnUpload, &QPushButton::clicked, this, [=]() {
        QString filePath = QFileDialog::getOpenFileName(this,
                                                        "Select Avatar",
                                                        "",
                                                        "Images (*.png *.jpg *.jpeg)");
        if (!filePath.isEmpty()) {
            m_avatarPath = filePath;
            QPixmap pix(filePath);
            if (!pix.isNull()) {
                int size = 100;
                QPixmap target(size, size);
                target.fill(Qt::transparent);

                QPixmap scaledSrc = pix.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

                QPainter painter(&target);
                painter.setRenderHint(QPainter::Antialiasing, true);
                painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

                QPainterPath path;
                path.addEllipse(0, 0, size, size);
                painter.setClipPath(path);

                int x = (scaledSrc.width() - size) / 2;
                int y = (scaledSrc.height() - size) / 2;
                painter.drawPixmap(0, 0, scaledSrc, x, y, size, size);

                lblAvatarPreview->setPixmap(target);
            }
        }
    });

    // Add rows to form
    form->addRow(makeLabel("Avatar"),         avatarLayout); // Add Avatar to form
    form->addRow(makeLabel("Full Name *"),    inpName);
    form->addRow(makeLabel("Role *"),         cmbRole);
    form->addRow(makeLabel("Phone"),          inpPhone);
    form->addRow(makeLabel("Date of Birth"),  inpDob);
    form->addRow(makeLabel("Address"),        inpAddress);
    form->addRow(makeLabel("Citizen ID"),     inpCitizenId);

    // Separator for account section
    QLabel *lblAccount = new QLabel("— Account Credentials —");
    lblAccount->setObjectName("lblAccount");
    lblAccount->setAlignment(Qt::AlignCenter);
    form->addRow(lblAccount);

    form->addRow(makeLabel("Username *"),  inpUsername);
    form->addRow(makeLabel("Password *"),  inpPassword);

    mainLayout->addLayout(form);

    //Error label
    lblError = new QLabel();
    lblError->setObjectName("lblError");
    lblError->setWordWrap(true);
    lblError->setVisible(false);
    mainLayout->addWidget(lblError);

    //Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    btnCancel = new QPushButton("Cancel");
    btnCancel->setObjectName("btnCancel");
    btnCancel->setMinimumHeight(36);
    btnCancel->setCursor(Qt::PointingHandCursor);

    btnConfirm = new QPushButton("Add Employee");
    btnConfirm->setObjectName("btnConfirm");
    btnConfirm->setMinimumHeight(36);
    btnConfirm->setCursor(Qt::PointingHandCursor);

    btnLayout->addStretch();
    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnConfirm);
    mainLayout->addLayout(btnLayout);

    //Connections
    connect(btnConfirm, &QPushButton::clicked, this, &AddEmployee_Dialog::onConfirm);
    connect(btnCancel,  &QPushButton::clicked, this, &QDialog::reject);
    QFile file(":/styles/Employee_View_styles.qss");
    if (file.open(QFile::ReadOnly)) {
        this->setStyleSheet(QLatin1String(file.readAll()));
        file.close();
    }
}

// Validate + Accept
bool AddEmployee_Dialog::validate()
{
    if (inpName->text().trimmed().isEmpty()) {
        lblError->setText("⚠  Full name is required.");
        lblError->setVisible(true);
        inpName->setFocus();
        return false;
    }
    if (inpUsername->text().trimmed().isEmpty()) {
        lblError->setText("⚠  Username is required.");
        lblError->setVisible(true);
        inpUsername->setFocus();
        return false;
    }
    if (inpPassword->text().isEmpty()) {
        lblError->setText("⚠  Password is required.");
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
    // close dialog immediately and flag QDialog::Accept
}

// ============================================================
// Getters
// ============================================================

QString AddEmployee_Dialog::getName()       const { return inpName->text().trimmed(); }
QString AddEmployee_Dialog::getRole()       const { return cmbRole->currentText(); }
QString AddEmployee_Dialog::getPhone()      const { return inpPhone->text().trimmed(); }
QString AddEmployee_Dialog::getDob()        const { return inpDob->text().trimmed(); }
QString AddEmployee_Dialog::getAddress()    const { return inpAddress->text().trimmed(); }
QString AddEmployee_Dialog::getCitizenId()  const { return inpCitizenId->text().trimmed(); }
QString AddEmployee_Dialog::getUsername()   const { return inpUsername->text().trimmed(); }
QString AddEmployee_Dialog::getPassword()   const { return inpPassword->text(); }
QString AddEmployee_Dialog::getAvatarPath() const { return m_avatarPath; } // Getter for avatar path