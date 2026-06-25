#include "Login_View.h"

Login_View::Login_View(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::Login_ViewClass())
{
	ui->setupUi(this);
}

Login_View::~Login_View()
{
	delete ui;
}

