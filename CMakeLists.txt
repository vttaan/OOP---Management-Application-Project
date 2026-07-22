cmake_minimum_required(VERSION 3.16)
project(OOP_Management_Application LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON) 
set(CMAKE_AUTORCC ON) 
set(CMAKE_AUTOUIC ON)

# set(CMAKE_PREFIX_PATH "C:/Qt/6.11.1/mingw_64")
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)


# LIST OF FILE CODE
set(PROJECT_SOURCES
    src/main.cpp
    resources/resources.qrc

    # file in Core
        # HEADER
        src/core/User.h
        src/core/Staff.h
        src/core/Manager.h
        src/core/UserFactory.h
        # CPP
        src/core/User.cpp
        src/core/Staff.cpp
        src/core/Manager.cpp
        src/core/UserFactory.cpp

    # file in Utils
        # HEADER
        src/utils/Database.h
        src/utils/SessionManage.h
        src/utils/Security.h
        src/utils/Config.h
        # CPP
        src/utils/Database.cpp
        src/utils/SessionManage.cpp
        src/utils/Security.cpp

    # file in Model
        # HEADER
        src/model/Login_Model.h
        src/model/Change_password.h
        src/model/Employee_Model.h
        src/model/Profile_Model.h
        src/model/validator.h
        src/model/Salary_Model.h
        # CPP
        src/model/Login_Model.cpp
        src/model/Change_password.cpp
        src/model/Employee_Model.cpp
        src/model/Profile_Model.cpp
        src/model/validator.cpp
        src/model/Salary_Model.cpp

    # file in Control
        # HEADER
        src/control/Login_Control.h
        src/control/Profile_Control.h
        src/control/Control_Navigator.h
        src/control/Dashboard_Control.h
        src/control/Employee_Control.h
        src/control/Salary_Control.h
        # CPP
        src/control/Login_Control.cpp
        src/control/Profile_Control.cpp
        src/control/Control_Navigator.cpp
        src/control/Dashboard_Control.cpp
        src/control/Employee_Control.cpp
        src/control/Salary_Control.cpp

    # file in View
        # HEADER
        src/view/Login_View.h
        src/view/Dashboard_View.h
        src/view/View_Navigator.h
        src/view/Profile_View.h
        src/view/employeeswidget.h
        src/view/EditProfile_Widget.h
        src/view/EditPassword_Widget.h
        src/view/employeecard.h
        src/view/AddEmployee_Dialog.h
        src/view/EditEmployee_Dialog.h
        src/view/Schedule_View.h
        src/view/sidebar_widget.h
        src/view/Salary_View.h
        # CPP
        src/view/Login_View.cpp
        src/view/Dashboard_View.cpp
        src/view/View_Navigator.cpp
        src/view/Profile_View.cpp
        src/view/employeeswidget.cpp
        src/view/EditProfile_Widget.cpp
        src/view/EditPassword_Widget.cpp
        src/view/employeecard.cpp
        src/view/AddEmployee_Dialog.cpp
        src/view/EditEmployee_Dialog.cpp
        src/view/Schedule_View.cpp
        src/view/sidebar_widget.cpp
        src/view/Salary_View.cpp
        # UI
        src/view/Login_View.ui
        src/view/Dashboard_View.ui
        src/view/View_Navigator.ui
        src/view/Profile_View.ui
        src/view/employeeswidget.ui
        src/view/employeecard.ui
        src/view/Schedule_View.ui
        src/view/sidebar_widget.ui
        src/view/Salary_View.ui
)

qt_add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES}
    src/view/Dashboard_View.h src/view/Dashboard_View.cpp src/view/Dashboard_View.ui
    src/view/employeecard.h src/view/employeecard.cpp src/view/employeecard.ui

    src/view/Profile_View.h src/view/Profile_View.cpp src/view/Profile_View.ui
    src/view/View_Navigator.h src/view/View_Navigator.cpp src/view/View_Navigator.ui
    src/view/employeeswidget.h
    src/control/Profile_Control.h src/control/Profile_Control.cpp
    src/control/Control_Navigator.h src/control/Control_Navigator.cpp
    src/control/Dashboard_Control.h src/control/Dashboard_Control.cpp
    src/model/Employee_Model.cpp src/model/Employee_Model.h
    src/view/Schedule_View.h src/view/Schedule_View.cpp src/view/Schedule_View.ui
    src/core/Shift.h
    src/core/Shift.cpp
    src/model/Schedule_Model.h src/model/Schedule_Model.cpp
    src/control/Schedule_Control.h src/control/Schedule_Control.cpp
    src/view/overview_view.h src/view/overview_view.cpp src/view/overview_view.ui
    src/control/overview_control.h src/control/overview_control.cpp
    src/model/Validator.h src/model/Validator.cpp
    src/model/Salary_Model.h src/model/Salary_Model.cpp
    src/control/Salary_Control.h src/control/Salary_Control.cpp
    src/view/Salary_View.h src/view/Salary_View.cpp src/view/Salary_View.ui
    #src/utils/Config.h
    )
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_precompile_headers(${PROJECT_NAME} PRIVATE src/global.h)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/database 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
