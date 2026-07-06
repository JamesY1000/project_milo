cmake_minimum_required(VERSION 3.8)
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

# Include the 'include' directory
include_directories(include)

# Proto generated code
set(MILO_PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../milo_proto/generated_code)

add_executable(hardware_interface_node
  src/hardware_interface.cpp
  ${MILO_PROTO_DIR}/RoverCommand.pb.cc
  ${MILO_PROTO_DIR}/google/protobuf/timestamp.pb.cc
)

target_include_directories(hardware_interface_node PRIVATE
  ${MILO_PROTO_DIR}
)

# Install share directories
install(DIRECTORY
  config
  launch
  DESTINATION share/${PROJECT_NAME})

ament_target_dependencies(hardware_interface_node 
  rclcpp 
  std_msgs
  milo_interfaces
)

target_link_libraries(hardware_interface_node
  ${Protobuf_LIBRARIES}
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