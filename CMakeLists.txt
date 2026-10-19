cmake_minimum_required(VERSION 3.16)

project(rosvis VERSION 0.1 LANGUAGES CXX)

#
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THIRD_PARTY_ROOT "/home/f2tek/SwDev" CACHE PATH "Root of third-party libraries")
set(VULKAN_SDK_DEFAULT "${THIRD_PARTY_ROOT}/vulkansdk/x86_64")
set(VULKAN_SDK "${VULKAN_SDK_DEFAULT}" CACHE PATH "LunarG Vulkan SDK root")
set(ASSIMP_ROOT "${THIRD_PARTY_ROOT}/assimp" CACHE PATH "Assimp source or install directory")
set(MESHOPTIMIZER_ROOT "${THIRD_PARTY_ROOT}/meshoptimizer" CACHE PATH "meshoptimizer source directory")


#
find_path(ASSIMP_INCLUDE_DIR assimp/Importer.hpp HINTS "${ASSIMP_ROOT}/include" REQUIRED)
find_path(ASSIMP_CONFIG_INCLUDE_DIR assimp/config.h HINTS "${ASSIMP_ROOT}/build/include" "${ASSIMP_ROOT}/include" REQUIRED)
find_library(ASSIMP_LIBRARY NAMES assimp HINTS "${ASSIMP_ROOT}/build/bin" "${ASSIMP_ROOT}/lib" REQUIRED)

#
file(GLOB MESHOPTIMIZER_SOURCES CONFIGURE_DEPENDS "${MESHOPTIMIZER_ROOT}/src/*.cpp")
if(NOT MESHOPTIMIZER_SOURCES)
    message(FATAL_ERROR "Set MESHOPTIMIZER_ROOT to a meshoptimizer source checkout")
endif()
add_library(meshoptimizer STATIC ${MESHOPTIMIZER_SOURCES})
target_include_directories(meshoptimizer PUBLIC "${MESHOPTIMIZER_ROOT}/src")

#
find_package(Vulkan REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

#
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

qt_standard_project_setup(REQUIRES 6.12)

qt_add_executable(rosvis
    main.cpp
    renderer/renderer.cpp renderer/renderer.h
    ui/vulkanview.cpp ui/vulkanview.h
    renderer/shaders.h renderer/shaders.cpp renderer/scene.h renderer/scene.cpp renderer/mesh.h renderer/mesh.cpp
    ros2/uiadapter.h ros2/uiadapter.cpp ros2/rosadapter.h ros2/rosadapter.cpp ros2/data.h
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
    Boost::system
    Threads::Threads
)

target_include_directories(rosvis PRIVATE
    ui
)

set_source_files_properties(Style.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(rosvis
    URI ROSVis
    VERSION 1.0
    QML_FILES ui/DataTableView.qml ui/Main.qml ui/RosTopicsView.qml ui/Style.qml
    SOURCES ui/vulkanview.cpp ui/vulkanview.h
    RESOURCE_PREFIX /ros2
    NO_RESOURCE_TARGET_PATH
)

set(SHADER_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/shader)
set(SPIRV_DIR   ${CMAKE_CURRENT_BINARY_DIR}/shader)
file(GLOB SHADERS
    ${SHADER_DIR}/*.vert
    ${SHADER_DIR}/*.frag
    ${SHADER_DIR}/*.comp
    ${SHADER_DIR}/*.geom
    ${SHADER_DIR}/*.tesc
    ${SHADER_DIR}/*.tese
)
set(SPIRV_OUTPUTS "")
foreach(SHADER ${SHADERS})
    get_filename_component(NAME ${SHADER} NAME)
    set(SPIRV "${SPIRV_DIR}/${NAME}.spv")

    add_custom_command(
        OUTPUT  ${SPIRV}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SPIRV_DIR}
        COMMAND Vulkan::glslc
                --target-env=vulkan1.3
                -O
                ${SHADER} -o ${SPIRV}
        DEPENDS ${SHADER}
        COMMENT "Compiling SPIR-V: ${NAME}"
        VERBATIM
    )
    list(APPEND SPIRV_OUTPUTS ${SPIRV})
endforeach()
add_custom_target(shaders ALL DEPENDS ${SPIRV_OUTPUTS})
add_dependencies(rosvis shaders)

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
