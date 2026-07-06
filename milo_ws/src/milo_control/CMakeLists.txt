cmake_minimum_required(VERSION 3.8)
project(milo_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(milo_interfaces REQUIRED)

# Include the 'include' directory
include_directories(include)

# Install share directories
install(DIRECTORY
  config
  launch
  DESTINATION share/${PROJECT_NAME})

# Create executables
add_executable(controller_teleop_node src/controller_teleop.cpp)
ament_target_dependencies(controller_teleop_node
  rclcpp 
  std_msgs
  sensor_msgs
  geometry_msgs
  milo_interfaces
)

add_executable(rover_controller_node src/rover_controller.cpp)
ament_target_dependencies(rover_controller_node 
  rclcpp 
  std_msgs
  geometry_msgs
  milo_interfaces
)

# Install cpp executables
install(TARGETS 
  controller_teleop_node
  rover_controller_node 
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
