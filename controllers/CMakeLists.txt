add_library(AppController
    maincontroller.cpp
    maincontroller.h
    cameracontroller.h cameracontroller.cpp
)

target_include_directories(AppController PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_CURRENT_BINARY_DIR}/../view/AppView_autogen/include/"
)

add_library(MVCAMSDK SHARED IMPORTED)
set_target_properties(MVCAMSDK PROPERTIES
    IMPORTED_IMPLIB "C:\\Projects\\_Libs\\ChinaVision\\SDK\\X64\\MVCAMSDK_X64.lib"
    IMPORTED_LOCATION "C:\\Projects\\_Libs\\ChinaVision\\SDK\\X64\\MVCAMSDK_X64.dll"
    INTERFACE_INCLUDE_DIRECTORIES "C:\\Projects\\_Libs\\ChinaVision\\Demo\\VC++\\Include")

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)

target_link_libraries(AppController
    PUBLIC
        Qt::Core
        Qt::Widgets
        AppView
        MVCAMSDK
)
target_link_libraries(AppController PRIVATE Qt6::Core)
target_link_libraries(AppController PRIVATE Qt6::Core)

set_target_properties(AppController PROPERTIES
    AUTOMOC ON
    AUTOUIC OFF
    AUTORCC ON
)
