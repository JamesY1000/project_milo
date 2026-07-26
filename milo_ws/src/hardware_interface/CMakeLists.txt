cmake_minimum_required(VERSION 3.10)
project(hardware_interface)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(milo_interfaces REQUIRED)
find_package(Protobuf REQUIRED)

# Proto generated code
set(MILO_PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../milo_proto/generated_code)

add_executable(hardware_interface_node
  src/hardware_interface.cpp
  ${MILO_PROTO_DIR}/RoverCommand.pb.cc
  ${MILO_PROTO_DIR}/google/protobuf/timestamp.pb.cc
)

# Install share directories
install(DIRECTORY
  config
  launch
  DESTINATION share/${PROJECT_NAME})

target_include_directories(hardware_interface_node PRIVATE
  include
  ${MILO_PROTO_DIR}
)

target_link_libraries(hardware_interface_node
  PRIVATE
  rclcpp::rclcpp
  ${std_msgs_TARGETS}
  ${milo_interfaces_TARGETS}
  protobuf::libprotobuf
)

# Install cpp executables
install(TARGETS
  hardware_interface_node
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()