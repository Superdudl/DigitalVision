cmake_minimum_required(VERSION 3.19)
set(CMAKE_TOOLCHAIN_FILE "D:/Projects/_Libs/vcpkg/scripts/buildsystems/vcpkg.cmake")
project(DigitalVision LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_subdirectory(controllers/)
add_subdirectory(utils/)
add_subdirectory(view/)

set( OpenCV_DIR )
if( NOT CMAKE_CL_64 )
  set( OpenCV_DIR "D:/Projects/_Libs/vcpkg/installed/x86-windows/share/opencv4" )
else()
  set( OpenCV_DIR "D:/Projects/_Libs/vcpkg/installed/x64-windows/share/opencv4" )
endif()

set(quirc_DIR "D:/Projects/_Libs/vcpkg/installed/x64-windows/share/quirc")

find_package(OpenCV REQUIRED)
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(DigitalVision
    WIN32 MACOSX_BUNDLE
    main.cpp

)


include_directories($OpenCV_INCLUDE_DIRS})

target_link_libraries(DigitalVision
    PUBLIC
        Qt::Core
        Qt::Widgets
        ${OpenCV_LIBS}
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
