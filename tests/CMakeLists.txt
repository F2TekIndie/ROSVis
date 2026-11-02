cmake_minimum_required(VERSION 3.20)

project(rosvis_client_tests LANGUAGES CXX)

#
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)

#
find_package(Qt6 REQUIRED COMPONENTS Core)

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
