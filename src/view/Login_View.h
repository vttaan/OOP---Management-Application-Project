#pragma once

#include <QWidget>
#include "ui_Login_View.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Login_ViewClass; };
QT_END_NAMESPACE

class Login_View : public QWidget
{
	Q_OBJECT

public:
	Login_View(QWidget *parent = nullptr);
	~Login_View();

private:
	Ui::Login_ViewClass *ui;
};

