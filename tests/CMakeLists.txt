cmake_minimum_required(VERSION 3.20)

project(rosvis_client_tests LANGUAGES CXX)

#
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)

#
find_package(Qt6 REQUIRED COMPONENTS Core Qml Test)

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../../ROSData"
    rosdata
)

add_executable(grpc_client_test
    grpc_client_test.cpp
    ../ros2/rosadapter.cpp ../ros2/rosadapter.h
    ../ros2/commands.h
)

target_include_directories(grpc_client_test PRIVATE
    ../ros2
)

target_link_libraries(grpc_client_test PRIVATE
    Qt6::Core
    rosdata
)

enable_testing()
add_test(NAME grpc_client COMMAND grpc_client_test)
add_test(NAME grpc_existing_subscription COMMAND grpc_client_test --existing-subscription)
add_test(NAME grpc_schema_failure COMMAND grpc_client_test --schema-failure)

add_executable(rosdata_schema_test
    schema.cpp
)
target_link_libraries(rosdata_schema_test PRIVATE
    rosdata
)
add_test(NAME rosdata_schema COMMAND rosdata_schema_test)

add_executable(topicdata_model_test
    topicdatamodel.cpp
    ../ros2/topicdatamodel.cpp ../ros2/topicdatamodel.h
)
target_include_directories(topicdata_model_test PRIVATE ../ros2)
target_link_libraries(topicdata_model_test PRIVATE Qt6::Core Qt6::Qml Qt6::Test)
add_test(NAME topicdata_model COMMAND topicdata_model_test)

find_package(Qt6 REQUIRED COMPONENTS Gui Quick)
add_executable(topicdata_view_test
    data_view.cpp
    ../ros2/topicdatamodel.cpp ../ros2/topicdatamodel.h
)
target_include_directories(topicdata_view_test PRIVATE ../ros2)
target_compile_definitions(topicdata_view_test PRIVATE
    DATA_VIEW_SOURCE="${CMAKE_CURRENT_SOURCE_DIR}/../ui/DataTableView.qml"
)
target_link_libraries(topicdata_view_test PRIVATE Qt6::Gui Qt6::Quick Qt6::Test)
add_test(NAME topicdata_view COMMAND topicdata_view_test)
set_tests_properties(topicdata_view PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORMTHEME=;QT_QUICK_CONTROLS_STYLE=Basic")

add_executable(scene_camera_test
    camera.cpp
    ../renderer/scene.cpp
    ../renderer/camera.cpp ../renderer/camera.h ../renderer/light.h
    ../renderer/mesh.cpp
    ../renderer/meshfactory.cpp
)
target_include_directories(scene_camera_test PRIVATE ../renderer)
target_link_libraries(scene_camera_test PRIVATE Qt6::Gui Qt6::Test)
add_test(NAME scene_camera COMMAND scene_camera_test)
