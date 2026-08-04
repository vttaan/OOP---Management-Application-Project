cmake_minimum_required(VERSION 3.16)
project(OOP_Management_Application LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON) 
set(CMAKE_AUTORCC ON) 
set(CMAKE_AUTOUIC ON)

# set(CMAKE_PREFIX_PATH "C:/Qt/6.11.1/mingw_64")
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Charts)
include(CTest)


# LIST OF FILE CODE
set(PROJECT_SOURCES
    src/main.cpp
    resources/resources.qrc

    # file in Core
        # HEADER
        src/core/User.h
        src/core/Staff.h
        src/core/Manager.h
        src/core/Optimizer.h
        src/core/UserFactory.h
        src/core/ShiftBlock.h
        src/core/Optimizer.h
        src/core/KitchenAssistant.h
        src/core/HallStaff.h
        src/core/Cashier.h
        src/core/userprototyperegistry.h
        # CPP
        src/core/User.cpp
        src/core/Staff.cpp
        src/core/Manager.cpp
        src/core/UserFactory.cpp
        src/core/Optimizer.cpp
        src/core/userprototyperegistry.cpp

    # file in Utils
        # HEADER
        src/utils/Database.h
        src/utils/SessionManage.h
        src/utils/Security.h
        src/utils/Config.h
        src/utils/ScheduleDTOs.h
        src/utils/NotificationDTOs.h
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
        src/model/Dashboard_Model.h
        src/model/Notification_Model.h
        src/model/LeaveRequest_Model.h
        # CPP
        src/model/Login_Model.cpp
        src/model/Change_password.cpp
        src/model/Employee_Model.cpp
        src/model/Profile_Model.cpp
        src/model/validator.cpp
        src/model/Salary_Model.cpp
        src/model/Dashboard_Model.cpp
        src/model/Notification_Model.cpp
        src/model/LeaveRequest_Model.cpp

    # file in Control
        # HEADER
        src/control/Login_Control.h
        src/control/Profile_Control.h
        src/control/Control_Navigator.h
        src/control/Dashboard_Control.h
        src/control/Employee_Control.h
        src/control/Salary_Control.h
        src/control/Notification_Control.h
        # CPP
        src/control/Login_Control.cpp
        src/control/Profile_Control.cpp
        src/control/Control_Navigator.cpp
        src/control/Dashboard_Control.cpp
        src/control/Employee_Control.cpp
        src/control/Salary_Control.cpp
        src/control/Notification_Control.cpp

    # file in View
        # HEADER
        src/view/Login_View.h
        src/view/Dashboard_View.h
        src/view/View_Navigator.h
        src/view/Profile_View.h
        src/view/Employee_View.h
        src/view/EditProfile_Widget.h
        src/view/EditPassword_Widget.h
        src/view/employeecard.h
        src/view/AddEmployee_Dialog.h
        src/view/EditEmployee_Dialog.h
        src/view/EmployeeDetails_Dialog.h
        src/view/Schedule_View.h
        src/view/sidebar_widget.h
        src/view/Salary_View.h
        src/view/Notification_View.h
        # CPP
        src/view/Login_View.cpp
        src/view/Dashboard_View.cpp
        src/view/View_Navigator.cpp
        src/view/Profile_View.cpp
        src/view/Employee_View.cpp
        src/view/EditProfile_Widget.cpp
        src/view/EditPassword_Widget.cpp
        src/view/employeecard.cpp
        src/view/AddEmployee_Dialog.cpp
        src/view/EditEmployee_Dialog.cpp
        src/view/EmployeeDetails_Dialog.cpp
        src/view/Schedule_View.cpp
        src/view/Schedule_View_Manager.cpp
        src/view/Schedule_View_Staff.cpp
        src/view/sidebar_widget.cpp
        src/view/Salary_View.cpp
        src/view/Notification_View.cpp
        # UI
        src/view/Login_View.ui
        src/view/Dashboard_View.ui
        src/view/View_Navigator.ui
        src/view/Profile_View.ui
        src/view/Employee_View.ui
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
    src/view/Employee_View.h
    src/control/Profile_Control.h src/control/Profile_Control.cpp
    src/control/Control_Navigator.h src/control/Control_Navigator.cpp
    src/control/Dashboard_Control.h src/control/Dashboard_Control.cpp
    src/model/Employee_Model.cpp src/model/Employee_Model.h
    src/view/Schedule_View.h src/view/Schedule_View.cpp src/view/Schedule_View.ui
    src/core/Shift.h
    src/core/Shift.cpp
    src/model/Schedule_Model.h src/model/Schedule_Model.cpp
    src/control/Schedule_Control.h src/control/Schedule_Control.cpp
    src/model/Validator.h src/model/Validator.cpp
    src/view/viewschedule_view.h src/view/viewschedule_view.cpp src/view/viewschedule_view.ui
    src/control/viewschedule_control.h src/control/viewschedule_control.cpp
    src/model/Salary_Model.h src/model/Salary_Model.cpp
    src/control/Salary_Control.h src/control/Salary_Control.cpp
    src/view/Salary_View.h src/view/Salary_View.cpp src/view/Salary_View.ui
    src/core/Optimizer.h src/core/Optimizer.cpp
    src/utils/Config.h
    src/utils/ScheduleDTOs.h
    src/core/KitchenAssistant.h
    src/core/HallStaff.h
    src/core/Cashier.h
    src/core/userprototyperegistry.h src/core/userprototyperegistry.cpp
    src/core/Cashier.cpp
    src/core/HallStaff.cpp
    src/core/KitchenAssistant.cpp
    src/view/Setting_View.h src/view/Setting_View.cpp src/view/Setting_View.ui
    src/model/Setting_Model.h src/model/Setting_Model.cpp
    src/control/Setting_Control.h src/control/Setting_Control.cpp
    src/utils/NotificationDTOs.h
    src/model/Notification_Model.h src/model/Notification_Model.cpp
    src/model/LeaveRequest_Model.h src/model/LeaveRequest_Model.cpp
    src/control/Notification_Control.h src/control/Notification_Control.cpp
    src/view/Notification_View.h src/view/Notification_View.cpp
    )
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql Qt6::Charts)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_precompile_headers(${PROJECT_NAME} PRIVATE src/global.h)
option(OPTIMUS_COPY_DATABASE "Copy the runtime SQLite database into the build tree" ON)
if(OPTIMUS_COPY_DATABASE)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/database
         DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    qt_add_executable(schedule_model_tests
        tests/SchedulePersistenceTests.cpp
        src/model/Login_Model.cpp
        src/model/Schedule_Model.cpp
        src/model/Salary_Model.cpp
        src/model/Notification_Model.cpp
        src/model/LeaveRequest_Model.cpp
        src/utils/Database.cpp
        src/utils/Security.cpp
        src/core/Shift.cpp
        src/core/User.cpp
        src/core/Staff.cpp
        src/core/Manager.cpp
        src/core/Cashier.cpp
        src/core/HallStaff.cpp
        src/core/KitchenAssistant.cpp
        src/core/UserFactory.cpp
        src/core/userprototyperegistry.cpp
        src/core/Optimizer.cpp
    )
    target_link_libraries(schedule_model_tests PRIVATE
        Qt6::Core Qt6::Widgets Qt6::Sql Qt6::Test)
    target_include_directories(schedule_model_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src)
    add_test(NAME schedule_model_persistence COMMAND schedule_model_tests)
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_RUNTIME_DIRECTORY "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    set_tests_properties(schedule_model_persistence PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QTFRAMEWORK_BYPASS_LICENSE_CHECK=1"
        ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:${QT_RUNTIME_DIRECTORY}")
endif()
