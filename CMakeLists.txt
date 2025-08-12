cmake_minimum_required(VERSION 3.19)
set(CMAKE_TOOLCHAIN_FILE "D:/Projects/_Libs/vcpkg/scripts/buildsystems/vcpkg.cmake")
project(DigitalVision LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_subdirectory(controllers/)
add_subdirectory(utils/)
add_subdirectory(view/)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(DigitalVision
    WIN32 MACOSX_BUNDLE
    main.cpp
)

target_link_libraries(DigitalVision
    PRIVATE
        Qt::Core
        AppView
        AppController
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
