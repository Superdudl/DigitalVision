cmake_minimum_required(VERSION 3.19)
set(CMAKE_TOOLCHAIN_FILE "" CACHE PATH "Path to vcpkg.cmake")
project(DigitalVision LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/view/")

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)
find_package(OpenCV REQUIRED)

add_library(MVCAMSDK SHARED IMPORTED)
set(MVCAMSDK_DIR "" CACHE PATH "Path to MVCAMSDK directory")

if (NOT MVCAMSDK_DIR)
    message(FATAL_ERROR "Set MVCAMSDK_DIR to MVCAMSDK directory")
endif()

set_target_properties(MVCAMSDK PROPERTIES
    IMPORTED_IMPLIB "${MVCAMSDK_DIR}/SDK/X64/MVCAMSDK_X64.lib"
    IMPORTED_LOCATION "${MVCAMSDK_DIR}/SDK/X64/MVCAMSDK_X64.dll"
    INTERFACE_INCLUDE_DIRECTORIES "${MVCAMSDK_DIR}/Demo/VC++/Include")

qt_standard_project_setup()

file(GLOB_RECURSE CONTROLLER_SOURCES "controllers/*.cpp" "controllers/*.h")
file(GLOB_RECURSE VIEW_SOURCES "view/*.cpp" "view/*.h" "view/*.ui")
file(GLOB_RECURSE UTILS_SOURCES "utils/*.cpp" "utils/*.h")

qt_add_executable(DigitalVision
    WIN32 MACOSX_BUNDLE
    main.cpp
    ${CONTROLLER_SOURCES}
    ${VIEW_SOURCES}
    ${UTILS_SOURCES}
)

target_link_libraries(DigitalVision
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        ${OpenCV_LIBS}
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
