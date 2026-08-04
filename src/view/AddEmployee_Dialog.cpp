#include "global.h"
#include "AddEmployee_Dialog.h"
#include <QPainterPath>
#include <QFileDialog>
#include <QEvent>
#include <QScrollArea>

namespace {
    class CalendarEventFilter : public QObject {
    public:
        CalendarEventFilter(QDateEdit* de, QObject* parent = nullptr) : QObject(parent), m_dateEdit(de) {}
    protected:
        bool eventFilter(QObject* obj, QEvent* event) override {
            if (event->type() == QEvent::Show) {
                if (m_dateEdit->date() == QDate(1900, 1, 1)) {
                    QCalendarWidget* cal = qobject_cast<QCalendarWidget*>(obj);
                    if (cal) cal->setCurrentPage(2000, 1);
                }
            }
            return QObject::eventFilter(obj, event);
        }
    private:
        QDateEdit* m_dateEdit;
    };
}

AddEmployee_Dialog::AddEmployee_Dialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Thêm nhân viên mới");
    setMinimumWidth(420);
    setMinimumHeight(500);
    resize(460, 650);
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

    // Form Scroll Area
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    QWidget *scrollContent = new QWidget(scrollArea);
    scrollContent->setObjectName("scrollContent");
    scrollContent->setStyleSheet("QWidget#scrollContent { background: transparent; }");

    QFormLayout *form = new QFormLayout(scrollContent);
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    auto makeInput = [scrollContent](const QString &placeholder) -> QLineEdit* {
        QLineEdit *inp = new QLineEdit(scrollContent);
        inp->setPlaceholderText(placeholder);
        inp->setMinimumHeight(32);
        return inp;
    };

    auto makeLabel = [scrollContent](const QString &text) -> QLabel* {
        QLabel *lbl = new QLabel(text, scrollContent);
        lbl->setObjectName("formLabel");
        return lbl;
    };

    inpName      = makeInput("vd: Nguyễn Văn A");
    inpPhone     = makeInput("vd: 0901234567");

    inpDob = new QDateEdit(scrollContent);
    inpDob->setCalendarPopup(true);
    inpDob->setDisplayFormat("yyyy-MM-dd");
    inpDob->setMinimumDate(QDate(1900, 1, 1));
    inpDob->setMaximumDate(QDate::currentDate());
    inpDob->setSpecialValueText("----/--/--");
    inpDob->setDate(QDate(1900, 1, 1)); // This triggers the special value text
    inpDob->setMinimumHeight(32);
    inpDob->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    inpDob->setProperty("dateSelected", false);

    connect(inpDob, &QDateEdit::dateChanged, this, [this]() {
        inpDob->setProperty("dateSelected", true);
    });
    
    // Prevent manual text input, force using the calendar
    QLineEdit *leDob = inpDob->findChild<QLineEdit*>();
    if (leDob) leDob->setReadOnly(true);

    // Make the calendar year a spinbox without manual text input
    if (QCalendarWidget *cal = inpDob->calendarWidget()) {
        cal->setMinimumWidth(350); // Provide enough width for "Tháng Mười Một"
        if (QSpinBox *yearSpin = cal->findChild<QSpinBox*>()) {
            yearSpin->setReadOnly(true);
        }
        cal->installEventFilter(new CalendarEventFilter(inpDob, cal));

        // Add year navigation buttons to the calendar navigation bar
        QWidget *navBar = cal->findChild<QWidget*>("qt_calendar_navigationbar");
        if (navBar) {
            QHBoxLayout *navLayout = qobject_cast<QHBoxLayout*>(navBar->layout());
            if (navLayout) {
                QToolButton *prevYearBtn = new QToolButton(navBar);
                prevYearBtn->setText(QString::fromUtf8("\u00AB"));
                prevYearBtn->setAutoRaise(true);
                prevYearBtn->setFixedSize(28, 28);
                connect(prevYearBtn, &QToolButton::clicked, cal, [cal]() {
                    cal->showPreviousYear();
                });

                QToolButton *nextYearBtn = new QToolButton(navBar);
                nextYearBtn->setText(QString::fromUtf8("\u00BB"));
                nextYearBtn->setAutoRaise(true);
                nextYearBtn->setFixedSize(28, 28);
                connect(nextYearBtn, &QToolButton::clicked, cal, [cal]() {
                    cal->showNextYear();
                });

                navLayout->insertWidget(0, prevYearBtn);
                navLayout->addWidget(nextYearBtn);
            }
        }
    }

    inpAddress   = makeInput("vd: 123 Lê Lợi, TP.HCM");
    inpCitizenId = makeInput("vd: 012345678901");
    inpSalary    = makeInput("vd: 20000");

    cmbRole = new QComboBox(scrollContent);
    cmbRole->addItem("Thu ngân");
    cmbRole->addItem("Nhân viên sảnh");
    cmbRole->addItem("Phụ bếp");
    cmbRole->setMinimumHeight(32);

    cmbGender = new QComboBox(scrollContent);
    cmbGender->addItem("Nam");
    cmbGender->addItem("Nữ");
    cmbGender->addItem("Khác");
    cmbGender->setMinimumHeight(32);

    cmbIsFixedSalary = new QComboBox(scrollContent);
    cmbIsFixedSalary->addItem("Toàn thời gian (Cố định)");
    cmbIsFixedSalary->addItem("Bán thời gian (Theo giờ)");
    cmbIsFixedSalary->setMinimumHeight(32);

    // Avatar upload section
    lblAvatarPreview = new QLabel("Chưa có ảnh", scrollContent);
    lblAvatarPreview->setObjectName("lblAvatarPreview");
    lblAvatarPreview->setFixedSize(100, 100);
    lblAvatarPreview->setAlignment(Qt::AlignCenter);

    btnUpload = new QPushButton("Chọn ảnh...", scrollContent);
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
    form->addRow(makeLabel("Loại lương *"),     cmbIsFixedSalary);
    form->addRow(makeLabel("Lương(VNĐ) *"),          inpSalary);

    QLabel *lblAccount = new QLabel("— Thông tin tài khoản (Tự động cấp) —", scrollContent);
    lblAccount->setObjectName("lblAccount");
    lblAccount->setAlignment(Qt::AlignCenter);
    form->addRow(lblAccount);

    inpUsername = makeInput("");
    inpUsername->setReadOnly(true);
    inpUsername->setText("Chưa nhập đủ thông tin nhân viên");
    inpUsername->setStyleSheet("color: gray; background-color: #f2f2f2;");

    inpPassword = makeInput("");
    inpPassword->setReadOnly(true);

    inpPassword->setText("Chưa nhập đủ thông tin nhân viên");
    inpPassword->setStyleSheet("color: blue; background-color: #f2f2f2; font-weight: bold;");
    form->addRow(makeLabel("Tên đăng nhập"), inpUsername);
    form->addRow(makeLabel("Mật khẩu"),      inpPassword);

    auto updateAutoCredentials = [=]() {
        QString name = inpName->text().trimmed();
        QString dob = inpDob->date().toString("yyyy-MM-dd");

        // update user's password
        if (!name.isEmpty() && dob.length() == 10 && passwordGeneratorDelegate) {
            QString autoPass = passwordGeneratorDelegate(name, dob);
            if (!autoPass.isEmpty()) {
                inpPassword->setText(autoPass);
            }
        } else {
            inpPassword->setText("Đang chờ nhập đủ tên và ngày sinh...");
        }
        // update username
        if (!name.isEmpty() && usernameGeneratorDelegate) {
            QString role = getRole(); // (Manage // Staff // ....)
            QString autoUser = usernameGeneratorDelegate(role);
            inpUsername->setText(autoUser);
        } else {
            inpUsername->setText("Vui lòng nhập đủ thông tin cá nhân");
        }
    };

    connect(inpName, &QLineEdit::textChanged, this, updateAutoCredentials);
    connect(inpDob, &QDateEdit::dateChanged, this, updateAutoCredentials);


    connect(cmbRole, &QComboBox::currentTextChanged, this, updateAutoCredentials);

    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);

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
    if (validatorDelegate) {
        QString errorMsg = validatorDelegate(this); // Gọi Controller kiểm tra
        if (!errorMsg.isEmpty()) {
            lblError->setText(errorMsg);
            lblError->setVisible(true);
            return false;
        }
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
    else if (vn == "Thu ngân") return "Cashier";
    else if (vn == "Phụ bếp") return "KitchenAssistant";
    else if (vn == "Nhân viên sảnh") return "HallStaff";
    return "Staff";
}

QString AddEmployee_Dialog::getGender()     const { return cmbGender->currentText(); }
QString AddEmployee_Dialog::getPhone()      const { return inpPhone->text().trimmed(); }
bool AddEmployee_Dialog::isDobSelected()    const { return inpDob->property("dateSelected").toBool(); }
QString AddEmployee_Dialog::getDob()        const { return inpDob->date().toString("yyyy-MM-dd"); }
QString AddEmployee_Dialog::getAddress()    const { return inpAddress->text().trimmed(); }
QString AddEmployee_Dialog::getCitizenId()  const { return inpCitizenId->text().trimmed(); }
QString AddEmployee_Dialog::getAvatarPath() const { return m_avatarPath; }
int AddEmployee_Dialog::getSalary() const { return inpSalary->text().trimmed().toInt(); }
bool AddEmployee_Dialog::getIsFixedSalary() const { return cmbIsFixedSalary->currentIndex() == 0; }

// Auto-generated credentials — username and password from real input fields
QString AddEmployee_Dialog::getUsername()   const { return inpUsername->text().trimmed(); }
QString AddEmployee_Dialog::getPassword()   const { return inpPassword->text(); }