add_library(AppView
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui


)

target_include_directories(AppView PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

target_link_libraries(AppView
    PUBLIC
        Qt::Core
        Qt::Widgets
)

set_target_properties(AppView PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)
