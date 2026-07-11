#include "Dashboard_View.h"
#include "ui_Dashboard_View.h"
#include "control/Dashboard_Control.h"
#include <QVBoxLayout> //
Dashboard_View::Dashboard_View(Dashboard_Control *controller, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Dashboard_View),
    controller(controller),
    currentSidebar(nullptr)
{
    ui->setupUi(this);
}

Dashboard_View::~Dashboard_View()
{
    delete ui;
}

Dashboard_Control *Dashboard_View::getController() const {
    return controller;
}

void Dashboard_View::setController(Dashboard_Control *ctrl) {
    controller = ctrl;
}

void Dashboard_View::addSidebar(QWidget* sidebarWidget) {
    if (!sidebarWidget) return;
    currentSidebar = sidebarWidget;
    ui->horizontalLayout_Main->insertWidget(0, sidebarWidget);
}

void Dashboard_View::embedWidgetIntoPage(int index, QWidget* widget) {
    if (!widget) return;

    QWidget* targetPage = ui->stackedWidget->widget(index);
    if (targetPage) {
        if (!targetPage->layout()) {
            QVBoxLayout* newLayout = new QVBoxLayout(targetPage);
            newLayout->setContentsMargins(0, 0, 0, 0);
            targetPage->setLayout(newLayout);
        }

        targetPage->layout()->addWidget(widget); // Cục đá đã được dọn sạch!
    }
}

void Dashboard_View::switchPage(int index) {
    ui->stackedWidget->setCurrentIndex(index);
}

void Dashboard_View::toggleSidebar() {
    if (currentSidebar) {
        if (currentSidebar->isVisible()) {
            currentSidebar->hide();
        } else {
            currentSidebar->show();
        }
    }
}