#include "EditEmployee_Dialog.h"
#include <QFileDialog>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>

EditEmployee_Dialog::EditEmployee_Dialog(User *emp, QWidget *parent)
    : QDialog(parent)
{
    setObjectName("EditEmployee_Dialog");
    setWindowTitle("Edit Employee");
    setMinimumWidth(420);
    setModal(true);
    setupUi(emp);
}



void EditEmployee_Dialog::setupUi(User *emp)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(28, 24, 28, 24);

    // --- Title ---
    QLabel *lblTitle = new QLabel("Edit Employee");
    lblTitle->setObjectName("dlgTitle");
    mainLayout->addWidget(lblTitle);

    QLabel *lblSub = new QLabel("Update the employee information below.");
    lblSub->setObjectName("dlgSub");
    mainLayout->addWidget(lblSub);

    QFrame *divider = new QFrame();
    divider->setObjectName("dlgDivider");
    divider->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(divider);

    // --- Form ---
    QFormLayout *form = new QFormLayout();
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    auto makeInput = [&](const QString &placeholder, const QString &value) -> QLineEdit* {
        QLineEdit *inp = new QLineEdit(value);   // pre-fill value
        inp->setPlaceholderText(placeholder);
        inp->setMinimumHeight(32);
        return inp;
    };
    QFile file(":/styles/Employee_View_styles.qss");
    if (file.open(QFile::ReadOnly)) {
        this->setStyleSheet(QLatin1String(file.readAll()));
        file.close();
    }
    auto makeLabel = [&](const QString &text) -> QLabel* {
        QLabel *lbl = new QLabel(text);
        lbl->setObjectName("formLabel");
        return lbl;
    };


    inpName      = makeInput("e.g. Nguyen Van A",            emp ? emp->getName()        : "");
    inpPhone     = makeInput("e.g. 0901234567",              emp ? emp->getPhoneNum()    : "");
    inpDob       = makeInput("YYYY-MM-DD",                   emp ? emp->getDOB()         : "");
    inpAddress   = makeInput("e.g. 123 Le Loi, Ho Chi Minh", emp ? emp->getAddress()     : "");
    inpCitizenId = makeInput("e.g. 012345678901",            emp ? emp->getIndentityID() : "");

    cmbRole = new QComboBox();
    cmbRole->addItem("Staff");
    cmbRole->addItem("Manager");
    cmbRole->setMinimumHeight(32);


    if (emp) {
        int idx = cmbRole->findText(emp->getRole());
        if (idx >= 0) cmbRole->setCurrentIndex(idx);
    }

    //AVATAR UPLOAD SECTION
    lblAvatarPreview = new QLabel("No Avatar");
    lblAvatarPreview->setObjectName("lblAvatarPreview");
    lblAvatarPreview->setFixedSize(100, 100);
    lblAvatarPreview->setAlignment(Qt::AlignCenter);

    m_avatarPath = emp ? emp->getAvatarPath() : "";
    if (!m_avatarPath.isEmpty()) {
        QPixmap pix(m_avatarPath);
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

    btnUpload = new QPushButton("Browse...");
    btnUpload->setObjectName("btnUpload");
    btnUpload->setCursor(Qt::PointingHandCursor);

    QHBoxLayout *avatarLayout = new QHBoxLayout();
    avatarLayout->addWidget(lblAvatarPreview);
    avatarLayout->addWidget(btnUpload);
    avatarLayout->addStretch();

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

    form->addRow(makeLabel("Avatar"),        avatarLayout);
    form->addRow(makeLabel("Full Name *"),   inpName);
    form->addRow(makeLabel("Role *"),        cmbRole);
    form->addRow(makeLabel("Phone"),         inpPhone);
    form->addRow(makeLabel("Date of Birth"), inpDob);
    form->addRow(makeLabel("Address"),       inpAddress);
    form->addRow(makeLabel("Citizen ID"),    inpCitizenId);
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

    btnConfirm = new QPushButton("Save Changes");
    btnConfirm->setObjectName("btnConfirm");
    btnConfirm->setMinimumHeight(36);
    btnConfirm->setCursor(Qt::PointingHandCursor);

    btnLayout->addStretch();
    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnConfirm);
    mainLayout->addLayout(btnLayout);

    connect(btnConfirm, &QPushButton::clicked, this, &EditEmployee_Dialog::onConfirm);
    connect(btnCancel,  &QPushButton::clicked, this, &QDialog::reject);
}

bool EditEmployee_Dialog::validate()
{
    if (inpName->text().trimmed().isEmpty()) {
        lblError->setText("⚠  Full name is required.");
        lblError->setVisible(true);
        inpName->setFocus();
        return false;
    }
    lblError->setVisible(false);
    return true;
}

void EditEmployee_Dialog::onConfirm()
{
    if (validate()) accept();
}


// Getters
QString EditEmployee_Dialog::getName()      const { return inpName->text().trimmed(); }
QString EditEmployee_Dialog::getRole()      const { return cmbRole->currentText(); }
QString EditEmployee_Dialog::getPhone()     const { return inpPhone->text().trimmed(); }
QString EditEmployee_Dialog::getDob()       const { return inpDob->text().trimmed(); }
QString EditEmployee_Dialog::getAddress()   const { return inpAddress->text().trimmed(); }
QString EditEmployee_Dialog::getCitizenId() const { return inpCitizenId->text().trimmed(); }
QString EditEmployee_Dialog::getAvatarPath() const { return m_avatarPath; }
