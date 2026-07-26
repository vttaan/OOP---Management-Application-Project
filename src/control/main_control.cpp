#include "global.h"
#include "main_control.h"

Main_Control::Main_Control(QObject *parent) : QObject(parent)
{
    // Không khởi tạo view ở đây nữa để tránh lỗi "invalid use of class"
}

Main_Control::~Main_Control()
{
}

void Main_Control::init()
{
    // No-op: Việc bơm dữ liệu và điều hướng đã có Control_Navigator lo!
}