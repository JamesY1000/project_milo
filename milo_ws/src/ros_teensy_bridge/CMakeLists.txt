cmake_minimum_required(VERSION 3.8)
project(ros_serial_bridge)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(milo_interfaces REQUIRED)
find_package(milo_proto REQUIRED)
find_package(Protobuf REQUIRED)

# Install share directories
install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME})

# Create executables
add_executable(ros_serial_bridge_node
  src/ros_serial_bridge.cpp
  ../milo_proto/generated_code/RoverCommand.pb.cc
  ../milo_proto/generated_code/google/protobuf/timestamp.pb.cc
  )

ament_target_dependencies(ros_serial_bridge_node 
  rclcpp 
  std_msgs
  geometry_msgs
  milo_interfaces
  milo_proto
  Protobuf
  )

target_include_directories(ros_serial_bridge_node PRIVATE
  ${Protobuf_INCLUDE_DIRS}
  ../milo_proto/generated_code
  ../milo_proto/generated_code/google/protobuf
  include
)

# Install cpp executables
install(TARGETS 
  ros_serial_bridge_node 
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
