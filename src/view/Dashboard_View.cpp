#include "Dashboard_View.h"
#include "ui_Dashboard_View.h"
#include "control/Dashboard_Control.h"
#include "employeecard.h"

// ---------------------------------------------------------------------------
// Helper: create a white rounded card with a title header and inner layout
// ---------------------------------------------------------------------------
QFrame* Dashboard_View::makeCard(const QString& title, QLayout* innerLayout, bool isDark)
{
    QFrame* card = new QFrame();
    card->setObjectName("mainCard");
    card->setStyleSheet(
        "QFrame#mainCard { background-color: #ffffff; border-radius: 16px; border: 1px solid #eef0f4; }"
    );

    QVBoxLayout* vl = new QVBoxLayout(card);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(0);

    if (!title.isEmpty()) {
        QFrame* header = new QFrame();
        header->setStyleSheet(
            isDark 
            ? "QFrame { background-color: #1a73e8; border-top-left-radius: 16px; border-top-right-radius: 16px; }"
            : "QFrame { background-color: transparent; }"
        );
        QVBoxLayout* headerLayout = new QVBoxLayout(header);
        headerLayout->setContentsMargins(18, 16, 18, 16);

        QLabel* lbl = new QLabel(title);
        QString textColor = isDark ? "#ffffff" : "#212b36";
        lbl->setStyleSheet(
            QString("font-size: 15px; font-weight: 700; color: %1; border: none; background: transparent;").arg(textColor)
        );
        if (isDark) {
            lbl->setAlignment(Qt::AlignCenter);
        }
        headerLayout->addWidget(lbl);
        vl->addWidget(header);

        QFrame* sep = new QFrame();
        sep->setFrameShape(QFrame::HLine);
        sep->setStyleSheet("border: none; border-top: 1px solid #eef0f4; background: transparent;");
        sep->setFixedHeight(1);
        vl->addWidget(sep);
    }

    if (innerLayout) {
        QWidget* bodyWrapper = new QWidget();
        bodyWrapper->setStyleSheet("background: transparent;");
        QVBoxLayout* bodyLayout = new QVBoxLayout(bodyWrapper);
        bodyLayout->setContentsMargins(18, 16, 18, 16);
        bodyLayout->addLayout(innerLayout);
        vl->addWidget(bodyWrapper);
    }
    return card;
}

// ---------------------------------------------------------------------------
// Constructor: build the 2x2 grid programmatically
// ---------------------------------------------------------------------------
Dashboard_View::Dashboard_View(Dashboard_Control *controller, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Dashboard_View)
    , controller(controller)
{
    ui->setupUi(this);

    // Legacy widgets were removed from the .ui file, so no need to hide them anymore.

    // -- Panel 1 (Top-Left): Employees in current shift ---------------------
    ui->scrollAreaEmployees->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollArea > QWidget > QWidget { background: transparent; }"
    );
    // Restrict horizontal scrolling - only allow vertical
    ui->scrollAreaEmployees->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollAreaEmployees->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->scrollAreaEmployees->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->scrollAreaEmployees->setWidgetResizable(true);

    m_empGridWidget = ui->scrollAreaWidgetContents;
    m_empGridWidget->setStyleSheet("background: transparent;");

    QVBoxLayout* empLayout = new QVBoxLayout();
    empLayout->setContentsMargins(0, 0, 0, 0);
    empLayout->addWidget(ui->scrollAreaEmployees);
    QFrame* frame1 = makeCard("Nhân Viên Trong Ca Hiện Tại", empLayout);

    // -- Panel 2 (Top-Right): Next shift employees ---------------------------
    m_nextShiftLayout = new QVBoxLayout();
    m_nextShiftLayout->setContentsMargins(0, 0, 0, 0);
    m_nextShiftLayout->setSpacing(6);
    m_nextShiftLayout->addStretch();

    QScrollArea* scrollNext = new QScrollArea();
    scrollNext->setWidgetResizable(true);
    scrollNext->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    QWidget* nextWrapper = new QWidget();
    nextWrapper->setStyleSheet("background: transparent;");
    nextWrapper->setLayout(m_nextShiftLayout);
    scrollNext->setWidget(nextWrapper);

    QVBoxLayout* nextOuter = new QVBoxLayout();
    nextOuter->setContentsMargins(0, 0, 0, 0);
    nextOuter->addWidget(scrollNext);
    QFrame* frame2 = makeCard(QString::fromUtf8("Ca Làm Tiếp Theo"), nextOuter, true);

    // -- Panel 4 (Bottom-Right): Absent employees ----------------------------
    m_absentLayout = new QVBoxLayout();
    m_absentLayout->setContentsMargins(0, 0, 0, 0);
    m_absentLayout->setSpacing(6);
    m_absentLayout->addStretch();

    QScrollArea* scrollAbsent = new QScrollArea();
    scrollAbsent->setWidgetResizable(true);
    scrollAbsent->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    QWidget* absentWrapper = new QWidget();
    absentWrapper->setStyleSheet("background: transparent;");
    absentWrapper->setLayout(m_absentLayout);
    scrollAbsent->setWidget(absentWrapper);

    QVBoxLayout* absentOuter = new QVBoxLayout();
    absentOuter->setContentsMargins(0, 0, 0, 0);
    absentOuter->addWidget(scrollAbsent);
    QFrame* frame3 = makeCard(QString::fromUtf8("Nhân Viên Nghỉ"), absentOuter, true);

    // -- Panel 3 (Bottom-Left): Salary bar chart with year tabs --------------
    m_yearTabBar = new QTabBar();
    m_yearTabBar->setStyleSheet(
        "QTabBar::tab {"
        "  padding: 4px 12px; font-size: 11px;"
        "  border-radius: 4px; border: 1px solid #dde3ed;"
        "  background: #f4f6f8; color: #637381; margin-right: 3px;"
        "}"
        "QTabBar::tab:selected {"
        "  background: #1a73e8; color: #ffffff; border-color: #1a73e8;"
        "  font-weight: 600;"
        "}"
        "QTabBar { background: transparent; border: none; }"
    );
    connect(m_yearTabBar, &QTabBar::currentChanged, this, &Dashboard_View::onYearTabClicked);

    m_lblLastYearCount = new QLabel("Năm trước: --");
    m_lblThisYearCount = new QLabel("Năm hiện tại: --");
    m_lblLastYearCount->setStyleSheet(
        "color: #637381; font-size: 12px; border: none; background: transparent;");
    m_lblThisYearCount->setStyleSheet(
        "color: #1a73e8; font-weight: bold; font-size: 12px; border: none; background: transparent;");

    QHBoxLayout* chartHeader = new QHBoxLayout();
    chartHeader->addWidget(m_yearTabBar);
    chartHeader->addStretch();
    chartHeader->addWidget(m_lblLastYearCount);
    chartHeader->addSpacing(12);
    chartHeader->addWidget(m_lblThisYearCount);

    // Tooltip label (hidden by default, shown on bar hover)
    m_tooltipLabel = new QLabel();
    m_tooltipLabel->setStyleSheet(
        "background: #1a73e8; color: white; padding: 6px 12px;"
        "border-radius: 0px; font-size: 13px; font-weight: bold; border: none; text-transform: uppercase;");
    m_tooltipLabel->setAlignment(Qt::AlignCenter);
    m_tooltipLabel->hide();

    // Chart setup - white background to avoid the dark artifact
    m_chart = new QChart();
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
    m_chart->setBackgroundBrush(QBrush(Qt::white));
    m_chart->setBackgroundPen(Qt::NoPen);
    m_chart->setPlotAreaBackgroundBrush(QBrush(Qt::white));
    m_chart->setPlotAreaBackgroundVisible(true);
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    m_chart->legend()->setBackgroundVisible(false);

    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setBackgroundBrush(QBrush(Qt::white));
    m_chartView->setFrameShape(QFrame::NoFrame);
    m_chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* chartLayout = new QVBoxLayout();
    chartLayout->setContentsMargins(0, 0, 0, 0);
    chartLayout->setSpacing(8);
    chartLayout->addLayout(chartHeader);
    chartLayout->addWidget(m_tooltipLabel);
    chartLayout->addWidget(m_chartView, 1);

    m_salaryCard = makeCard("Thống Kê Lương Nhân Viên", chartLayout);
    m_salaryCard->setStyleSheet(
        "QFrame { background-color: #ffffff; border-radius: 16px; border: 1px solid #eef0f4; }");

    // -- Clear old layout from pageOverview and install 2-column grid --------
    QLayout* oldLayout = ui->pageOverview->layout();
    if (oldLayout) {
        QLayoutItem* item;
        while ((item = oldLayout->takeAt(0))) {
            if (item->widget() && item->widget() != ui->scrollAreaEmployees)
                item->widget()->hide();
            delete item;
        }
        delete oldLayout;
    }

    QGridLayout* grid = new QGridLayout(ui->pageOverview);
    grid->setContentsMargins(24, 24, 24, 24);
    grid->setSpacing(16);
    grid->setColumnStretch(0, 75);
    grid->setColumnStretch(1, 25);
    grid->setRowStretch(0, 4);
    grid->setRowStretch(1, 6);

    grid->addWidget(frame1, 0, 0);
    grid->addWidget(frame2, 0, 1);
    grid->addWidget(m_salaryCard, 1, 0);
    grid->addWidget(frame3, 1, 1);
}

Dashboard_View::~Dashboard_View() { delete ui; }

// ---------------------------------------------------------------------------
// Year tab clicked -> emit signal to Controller
// ---------------------------------------------------------------------------
void Dashboard_View::onYearTabClicked(int index)
{
    if (index >= 0 && index < m_availableYears.size())
        emit yearChanged(m_availableYears[index]);
}



void Dashboard_View::setSalaryChartVisible(bool visible)
{
    if (m_salaryCard) {
        m_salaryCard->setVisible(visible);
    }
}

// ---------------------------------------------------------------------------
// Bar hovered -> show tooltip with month total in VND
// ---------------------------------------------------------------------------
void Dashboard_View::onBarHovered(bool status, int index, QBarSet* barSet)
{
    if (!status || !barSet) {
        m_tooltipLabel->hide();
        return;
    }
    double val = barSet->at(index);
    QString formatted;
    if (val >= 1000000000) {
        formatted = QString::number(val / 1000000000.0, 'f', 1) + " tỷ";
    } else if (val >= 1000000) {
        formatted = QString::number(val / 1000000.0, 'f', 1) + " triệu";
    } else {
        formatted = QLocale(QLocale::Vietnamese, QLocale::Vietnam).toString((qint64)val) + " VNĐ";
    }

    QString monthNames[] = {"T1","T2","T3","T4","T5","T6","T7","T8","T9","T10","T11","T12"};
    QString month = (index >= 0 && index < 12) ? monthNames[index] : "";
    QString text = barSet->label() + " " + month + ": " + formatted;
    m_tooltipLabel->setText(text.toUpper());
    m_tooltipLabel->show();
}

// ---------------------------------------------------------------------------
// Panel 1: clear all employee cards
// ---------------------------------------------------------------------------
void Dashboard_View::clearEmployeeGrid()
{
    QLayout* old = m_empGridWidget->layout();
    if (!old) return;
    QLayoutItem* item;
    while ((item = old->takeAt(0))) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    delete old;
}

// ---------------------------------------------------------------------------
// Panel 1: add a single employee card in a 4-column grid (auto-wraps down)
// ---------------------------------------------------------------------------
void Dashboard_View::addEmployeeCard(EmployeeCard* card)
{
    QGridLayout* grid = qobject_cast<QGridLayout*>(m_empGridWidget->layout());
    const int COLS = 4;
    if (!grid) {
        grid = new QGridLayout(m_empGridWidget);
        grid->setContentsMargins(12, 12, 12, 12);
        grid->setSpacing(16);
        grid->setAlignment(Qt::AlignTop);
        for (int i = 0; i < COLS; ++i) {
            grid->setColumnStretch(i, 1);
        }
    }
    int count = grid->count();
    
    card->setMinimumWidth(150);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    grid->addWidget(card, count / COLS, count % COLS);
}

// ---------------------------------------------------------------------------
// Panel 2: render next-shift employee rows with avatar initial + info
// ---------------------------------------------------------------------------
void Dashboard_View::updateNextShiftPanel(const QList<ShiftEmployeeInfo>& entries)
{
    // Remove all except trailing stretch
    while (m_nextShiftLayout->count() > 1) {
        QLayoutItem* i = m_nextShiftLayout->takeAt(0);
        if (i->widget()) i->widget()->deleteLater();
        delete i;
    }

    if (entries.isEmpty()) {
        QLabel* lbl = new QLabel(QString::fromUtf8("Không có nhân viên ca tiếp theo."));
        lbl->setStyleSheet(
            "color:#919eab; font-style:italic; padding:4px 0;"
            "background:transparent; border:none;");
        m_nextShiftLayout->insertWidget(0, lbl);
        return;
    }

    int idx = 0;
    for (const ShiftEmployeeInfo& e : entries) {
        QWidget* row = new QWidget();
        row->setStyleSheet("background: transparent;");
        row->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        QHBoxLayout* hl = new QHBoxLayout(row);
        hl->setContentsMargins(4, 4, 4, 4);
        hl->setSpacing(10);

        // Avatar circle (image or initial letter)
        QLabel* avatar = new QLabel();
        avatar->setFixedSize(36, 36);
        QPixmap pix(e.avatarPath);
        if (!e.avatarPath.isEmpty() && !pix.isNull()) {
            QPixmap scaledPix = pix.scaled(36, 36, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            QPixmap circularPix(36, 36);
            circularPix.fill(Qt::transparent);
            QPainter painter(&circularPix);
            painter.setRenderHint(QPainter::Antialiasing);
            QPainterPath path;
            path.addEllipse(circularPix.rect());
            painter.setClipPath(path);
            int x = (36 - scaledPix.width()) / 2;
            int y = (36 - scaledPix.height()) / 2;
            painter.drawPixmap(x, y, scaledPix);
            
            avatar->setPixmap(circularPix);
            avatar->setStyleSheet("border: none;");
        } else {
            QString initial = e.name.isEmpty() ? "?" : QString(e.name[0]).toUpper();
            QString bg = e.role.contains("Manager", Qt::CaseInsensitive) ? "#9333ea" : "#1a73e8";
            avatar->setText(initial);
            avatar->setAlignment(Qt::AlignCenter);
            avatar->setStyleSheet(
                QString("background:%1; color:white; border-radius:18px;"
                        "font-weight:bold; font-size:14px; border:none;").arg(bg));
        }

        // Name + phone
        QLabel* lName = new QLabel(e.name);
        lName->setStyleSheet(
            "font-weight:600; font-size:13px; color:#212b36; background:transparent; border:none;");

        QLabel* lPhone = new QLabel(e.phone);
        lPhone->setStyleSheet(
            "font-size:11px; color:#637381; background:transparent; border:none;");

        QVBoxLayout* nameCol = new QVBoxLayout();
        nameCol->setSpacing(1);
        nameCol->addWidget(lName);
        nameCol->addWidget(lPhone);

        // Role badge
        QString displayRole = e.role;
        if (displayRole.compare("Cashier", Qt::CaseInsensitive) == 0) displayRole = QString::fromUtf8("Thu ngân");
        else if (displayRole.compare("HallStaff", Qt::CaseInsensitive) == 0) displayRole = QString::fromUtf8("Nhân viên sảnh");
        else if (displayRole.compare("KitchenAssistant", Qt::CaseInsensitive) == 0) displayRole = QString::fromUtf8("Phụ bếp");
        else if (displayRole.compare("Manager", Qt::CaseInsensitive) == 0) displayRole = QString::fromUtf8("Quản lý");
        
        QLabel* lRole = new QLabel(displayRole);
        lRole->setStyleSheet(
            "font-size:10px; color:#637381; background:transparent; border:none;");
        lRole->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        hl->addWidget(avatar);
        hl->addLayout(nameCol, 1);
        hl->addWidget(lRole);

        // Separator line
        QFrame* sep = new QFrame();
        sep->setFrameShape(QFrame::HLine);
        sep->setStyleSheet("border: none; border-top: 1px solid #f0f0f0; background: transparent;");
        sep->setFixedHeight(1);

        QWidget* wrapper = new QWidget();
        wrapper->setStyleSheet("background: transparent;");
        QVBoxLayout* wl = new QVBoxLayout(wrapper);
        wl->setContentsMargins(0, 0, 0, 0);
        wl->setSpacing(0);
        wl->addWidget(row);
        wl->addWidget(sep);

        m_nextShiftLayout->insertWidget(idx++, wrapper);
    }
}

// ---------------------------------------------------------------------------
// Panel 4: render absent employee rows
// ---------------------------------------------------------------------------
void Dashboard_View::updateAbsentPanel(const QList<ShiftEmployeeInfo>& entries)
{
    // Remove all except trailing stretch
    while (m_absentLayout->count() > 1) {
        QLayoutItem* i = m_absentLayout->takeAt(0);
        if (i->widget()) i->widget()->deleteLater();
        delete i;
    }

    if (entries.isEmpty()) {
        QLabel* lbl = new QLabel(QString::fromUtf8("Không có nhân viên nghỉ."));
        lbl->setStyleSheet(
            "color:#919eab; font-style:italic; padding:4px 0;"
            "background:transparent; border:none;");
        m_absentLayout->insertWidget(0, lbl);
        return;
    }

    int idx = 0;
    for (const ShiftEmployeeInfo& e : entries) {
        QWidget* row = new QWidget();
        row->setStyleSheet("background: transparent;");
        row->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        QHBoxLayout* hl = new QHBoxLayout(row);
        hl->setContentsMargins(4, 4, 4, 4);
        hl->setSpacing(10);

        // Avatar circle (image or initial letter)
        QLabel* avatar = new QLabel();
        avatar->setFixedSize(36, 36);
        QPixmap pix(e.avatarPath);
        if (!e.avatarPath.isEmpty() && !pix.isNull()) {
            QPixmap scaledPix = pix.scaled(36, 36, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            QPixmap circularPix(36, 36);
            circularPix.fill(Qt::transparent);
            QPainter painter(&circularPix);
            painter.setRenderHint(QPainter::Antialiasing);
            QPainterPath path;
            path.addEllipse(circularPix.rect());
            painter.setClipPath(path);
            int x = (36 - scaledPix.width()) / 2;
            int y = (36 - scaledPix.height()) / 2;
            painter.drawPixmap(x, y, scaledPix);
            
            avatar->setPixmap(circularPix);
            avatar->setStyleSheet("border: none;");
        } else {
            QString initial = e.name.isEmpty() ? "?" : QString(e.name[0]).toUpper();
            avatar->setText(initial);
            avatar->setAlignment(Qt::AlignCenter);
            avatar->setStyleSheet(
                QString("background:#8B2020; color:white; border-radius:18px;"
                        "font-weight:bold; font-size:14px; border:none;"));
        }

        // Name + phone
        QLabel* lName = new QLabel(e.name);
        lName->setStyleSheet(
            "font-weight:600; font-size:13px; color:#212b36; background:transparent; border:none;");

        QLabel* lPhone = new QLabel(e.phone);
        lPhone->setStyleSheet(
            "font-size:11px; color:#637381; background:transparent; border:none;");

        QVBoxLayout* nameCol = new QVBoxLayout();
        nameCol->setSpacing(1);
        nameCol->addWidget(lName);
        nameCol->addWidget(lPhone);

        // Role badge
        QString displayRole = e.role;
        if (displayRole.compare("Cashier", Qt::CaseInsensitive) == 0) displayRole = QString::fromUtf8("Thu ngân");
        else if (displayRole.compare("HallStaff", Qt::CaseInsensitive) == 0) displayRole = QString::fromUtf8("Nhân viên sảnh");
        else if (displayRole.compare("KitchenAssistant", Qt::CaseInsensitive) == 0) displayRole = QString::fromUtf8("Phụ bếp");
        else if (displayRole.compare("Manager", Qt::CaseInsensitive) == 0) displayRole = QString::fromUtf8("Quản lý");
        
        QLabel* lRole = new QLabel(displayRole);
        lRole->setStyleSheet(
            "font-size:10px; color:#637381; background:transparent; border:none;");
        lRole->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        hl->addWidget(avatar);
        hl->addLayout(nameCol, 1);
        hl->addWidget(lRole);

        // Separator line
        QFrame* sep = new QFrame();
        sep->setFrameShape(QFrame::HLine);
        sep->setStyleSheet("border: none; border-top: 1px solid #f0f0f0; background: transparent;");
        sep->setFixedHeight(1);

        QWidget* wrapper = new QWidget();
        wrapper->setStyleSheet("background: transparent;");
        QVBoxLayout* wl = new QVBoxLayout(wrapper);
        wl->setContentsMargins(0, 0, 0, 0);
        wl->setSpacing(0);
        wl->addWidget(row);
        wl->addWidget(sep);

        m_absentLayout->insertWidget(idx++, wrapper);
    }
}

// ---------------------------------------------------------------------------
// Panel 3: update chart bars and year tabs
// ---------------------------------------------------------------------------
void Dashboard_View::updateSalaryChart(const QVector<double>& lastYear, const QVector<double>& thisYear, int lastYearEmpCount, int thisYearEmpCount, int selectedYear)
{
    // Disconnect temporarily to avoid triggering onYearChanged
    disconnect(m_yearTabBar, &QTabBar::currentChanged, nullptr, nullptr);

    // Build year tab list once (2020 … current year)
    if (m_availableYears.isEmpty()) {
        int currentSystemYear = QDate::currentDate().year();
        for (int y = 2020; y <= currentSystemYear; ++y)
            m_availableYears.append(y);

        for (int y : m_availableYears)
            m_yearTabBar->addTab(QString::number(y));
        connect(m_yearTabBar, &QTabBar::currentChanged,
                this, &Dashboard_View::onYearTabClicked);

        int idx = m_availableYears.indexOf(selectedYear);
        if (idx >= 0) m_yearTabBar->setCurrentIndex(idx);
    } else {
        int idx = m_availableYears.indexOf(selectedYear);
        if (idx >= 0) m_yearTabBar->setCurrentIndex(idx);
        connect(m_yearTabBar, &QTabBar::currentChanged,
                this, &Dashboard_View::onYearTabClicked);
    }
    // Update employee count labels
    m_lblLastYearCount->setText(
        QString("Năm %1: %2 nhân viên").arg(selectedYear - 1).arg(lastYearEmpCount));
    m_lblThisYearCount->setText(
        QString("Năm %1: %2 nhân viên").arg(selectedYear).arg(thisYearEmpCount));

    // Rebuild chart series
    m_chart->removeAllSeries();
    for (auto ax : m_chart->axes()) m_chart->removeAxis(ax);

    QBarSet* setLast = new QBarSet(QString::number(selectedYear - 1));
    QBarSet* setThis = new QBarSet(QString::number(selectedYear));
    setLast->setColor(QColor("#b0c4de"));
    setThis->setColor(QColor("#1a73e8"));

    for (int i = 0; i < 12; ++i) {
        *setLast << lastYear[i];
        *setThis << thisYear[i];
    }

    QBarSeries* series = new QBarSeries();
    series->append(setLast);
    series->append(setThis);
    series->setLabelsVisible(false);

    // Connect hover signal for VND tooltip
    connect(series, &QBarSeries::hovered, this, &Dashboard_View::onBarHovered);

    m_chart->addSeries(series);

    QStringList months;
    for (int i = 1; i <= 12; ++i) months << QString("T%1").arg(i);

    QBarCategoryAxis* axX = new QBarCategoryAxis();
    axX->append(months);
    m_chart->addAxis(axX, Qt::AlignBottom);
    series->attachAxis(axX);

    // Tính toán max value để set Y axis
    double maxVal = 0;
    for (int i = 0; i < 12; ++i) {
        if (lastYear[i] > maxVal) maxVal = lastYear[i];
        if (thisYear[i] > maxVal) maxVal = thisYear[i];
    }
    
    if (maxVal == 0) {
        maxVal = 400000000;
    }
    
    long long step = 100000000;
    long long maxBound = ((static_cast<long long>(maxVal) / step) + 1) * step;
    
    // Nếu maxBound < 400tr thì tối thiểu là 400tr như người dùng ví dụ
    if (maxBound < 400000000) maxBound = 400000000;

    QCategoryAxis* axY = new QCategoryAxis();
    axY->setTitleText("VNĐ");
    
    for (long long v = 0; v <= maxBound; v += step) {
        QString label = QString::number(v);
        for (int j = label.length() - 3; j > 0; j -= 3) {
            label.insert(j, ".");
        }
        axY->append(label, v);
    }
    
    axY->setRange(0, maxBound);
    axY->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    
    m_chart->addAxis(axY, Qt::AlignLeft);
    series->attachAxis(axY);
}