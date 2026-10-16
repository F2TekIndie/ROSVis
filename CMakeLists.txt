cmake_minimum_required(VERSION 3.16)

project(rosvis VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(THIRD_PARTY_ROOT "/home/f2tek/SwDev" CACHE PATH "Root of third-party libraries")
set(VULKAN_SDK_DEFAULT "${THIRD_PARTY_ROOT}/vulkansdk/x86_64")
set(VULKAN_SDK "${VULKAN_SDK_DEFAULT}" CACHE PATH "LunarG Vulkan SDK root")

find_package(Vulkan REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

qt_standard_project_setup(REQUIRES 6.12)

qt_add_executable(rosvis
    main.cpp
    renderer/renderer.cpp renderer/renderer.h
    ui/vulkanview.cpp ui/vulkanview.h
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(rosvis PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.rosvis
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(rosvis PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Vulkan::Vulkan
)

target_include_directories(rosvis
    PRIVATE ui
)

set_source_files_properties(Style.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(rosvis
    URI ROSVis
    VERSION 1.0
    QML_FILES
    ui/DataTableView.qml ui/Main.qml ui/RosTopicsView.qml
    SOURCES
        ui/vulkanview.cpp ui/vulkanview.h
    RESOURCE_PREFIX /ros2
    NO_RESOURCE_TARGET_PATH
    QML_FILES ui/Style.qml
)

install(TARGETS rosvis
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET rosvis
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
