set(CMAKE_TOOLCHAIN_FILE "D:/Projects/_Libs/vcpkg/scripts/buildsystems/vcpkg.cmake")
add_library(AppController
    maincontroller.cpp
    maincontroller.h
    cameracontroller.h cameracontroller.cpp
    sharecontroller.h sharecontroller.cpp
)

target_include_directories(AppController PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_CURRENT_BINARY_DIR}/../view/AppView_autogen/include/"
)

add_library(MVCAMSDK SHARED IMPORTED)
set(MVCAMSDK_DIR "D:/Projects/_Libs/ChinaVision")
set_target_properties(MVCAMSDK PROPERTIES
    IMPORTED_IMPLIB "${MVCAMSDK_DIR}/SDK/X64/MVCAMSDK_X64.lib"
    IMPORTED_LOCATION "${MVCAMSDK_DIR}/SDK/X64/MVCAMSDK_X64.dll"
    INTERFACE_INCLUDE_DIRECTORIES "${MVCAMSDK_DIR}/Demo/VC++/Include")


set(OpenCV_DIR "D:/Projects/_Libs/vcpkg/installed/x64-windows/share/opencv4")
set(quirc_DIR "D:/Projects/_Libs/vcpkg/installed/x64-windows/share/quirc")

find_package(OpenCV REQUIRED)
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Gui)
find_package(Qt6 REQUIRED COMPONENTS Core)

target_link_libraries(AppController
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt6::Gui
    PUBLIC
        ${OpenCV_LIBS}
        AppView
        MVCAMSDK
)

set_target_properties(AppController PROPERTIES
    AUTOMOC ON
    AUTOUIC OFF
    AUTORCC ON
)
