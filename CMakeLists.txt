cmake_minimum_required(VERSION 3.8)
project(robot_nico)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(behavior_architecture REQUIRED)
find_package(behaviortree_cpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(yaml-cpp REQUIRED)

set(dependencies
  rclcpp
  rclcpp_lifecycle
  ament_index_cpp
  behavior_architecture
  behaviortree_cpp
  std_msgs
)

# Include directories
include_directories(
  include
)

# Robot Nico orchestrator library (for use with mission_executor)
add_library(${PROJECT_NAME}_orchestrator SHARED
  src/robot_nico_orchestrator.cpp
)
ament_target_dependencies(${PROJECT_NAME}_orchestrator ${dependencies})

# Robot Nico BT nodes plugin library
add_library(${PROJECT_NAME}_bt_nodes SHARED
  src/bt_nodes/bt_plugins.cpp
  src/bt_nodes/log_message_action.cpp
)
ament_target_dependencies(${PROJECT_NAME}_bt_nodes ${dependencies})

# Required for BehaviorTree.CPP plugin export
target_compile_definitions(${PROJECT_NAME}_bt_nodes PRIVATE BT_PLUGIN_EXPORT)

# Install libraries
install(TARGETS ${PROJECT_NAME}_orchestrator ${PROJECT_NAME}_bt_nodes
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# Robot Nico orchestrator executable (original/legacy main)
add_executable(robot_nico_main
  src/robot_nico_main.cpp
  src/robot_nico_orchestrator.cpp
  src/bt_nodes/log_message_action.cpp
)

ament_target_dependencies(robot_nico_main ${dependencies})
target_link_libraries(robot_nico_main yaml-cpp)

# Install executables
install(TARGETS
  robot_nico_main
  DESTINATION lib/${PROJECT_NAME}
)

# Install launch files
install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}
)

# Install behavior tree files
install(DIRECTORY
  behaviors
  DESTINATION share/${PROJECT_NAME}
)

# Install config files
install(DIRECTORY
  config
  DESTINATION share/${PROJECT_NAME}
)

# Install include directory
install(DIRECTORY include/
  DESTINATION include/
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
