cmake_minimum_required(VERSION 3.16)
project(OOP_Management_Application LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON) 
set(CMAKE_AUTORCC ON) 
set(CMAKE_AUTOUIC ON)

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
        # CPP
        src/utils/Database.cpp
        src/utils/SessionManage.cpp
        src/utils/Security.cpp

    # file in Model
        # HEADER
        src/model/Login_Model.h
        src/model/Profile_Model.h
        # CPP
        src/model/Login_Model.cpp
        src/model/Profile_Model.cpp

    # file in Control
        # HEADER
        src/control/Login_Control.h
        # CPP
        src/control/Login_Control.cpp

    # file in View
        # HEADER
        src/view/Login_View.h
        src/view/Dashboard_View.h
        src/view/View_Navigator.h
        src/view/Profile_View.h
<<<<<<< Updated upstream
=======
        src/view/employeeswidget.h
        src/view/EditProfile_Widget.h
>>>>>>> Stashed changes
        # CPP
        src/view/Login_View.cpp
        src/view/Dashboard_View.cpp
        src/view/View_Navigator.cpp
        src/view/Profile_View.cpp
<<<<<<< Updated upstream
=======
        src/view/employeeswidget.cpp
        src/view/EditProfile_Widget.cpp
>>>>>>> Stashed changes
        # UI
        src/view/Login_View.ui
        src/view/Dashboard_View.ui
        src/view/View_Navigator.ui
        src/view/Profile_View.ui
)

qt_add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES}
    src/view/Dashboard_View.h src/view/Dashboard_View.cpp src/view/Dashboard_View.ui
    src/view/employeecard.h src/view/employeecard.cpp src/view/employeecard.ui

    src/view/Profile_View.h src/view/Profile_View.cpp src/view/Profile_View.ui
    src/view/View_Navigator.h src/view/View_Navigator.cpp src/view/View_Navigator.ui
    src/control/Profile_Control.h src/control/Profile_Control.cpp
    src/control/Profile_Control.cpp
    src/control/Control_Navigator.h src/control/Control_Navigator.cpp
    src/control/Dashboard_Control.h src/control/Dashboard_Control.cpp
    src/view/EditPassword_Widget.h src/view/EditPassword_Widget.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql)
target_include_directories(${PROJECT_NAME} PRIVATE src)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/database 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
