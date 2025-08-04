cmake_minimum_required(VERSION 3.19)
project(DigitalVision LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(DigitalVision
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

add_library(MVCAMSDK SHARED IMPORTED)
set_target_properties(MVCAMSDK PROPERTIES
    IMPORTED_IMPLIB "D:\\Projects\\_Libs\\ChinaVision\\SDK\\X64\\MVCAMSDK_X64.lib"
    IMPORTED_LOCATION "D:\\Projects\\_Libs\\ChinaVision\\SDK\\X64\\MVCAMSDK_X64.dll"
    INTERFACE_INCLUDE_DIRECTORIES "D:\\Projects\\_Libs\\ChinaVision\\Demo\\VC++\\Include")

target_link_libraries(DigitalVision
    PUBLIC
        Qt::Core
        Qt::Widgets
        MVCAMSDK
)

include(GNUInstallDirs)

install(TARGETS DigitalVision
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET DigitalVision
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
