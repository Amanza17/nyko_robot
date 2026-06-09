// Copyright 2025 Your Name
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "rclcpp/rclcpp.hpp"
#include "lifecycle_msgs/msg/transition.hpp"
#include "ament_index_cpp/get_package_share_directory.hpp"

#include "behavior_architecture/config_parser.hpp"
#include "robot_nico/robot_nico_orchestrator.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  // Accept an optional config file argument; default to the installed dummy_mission.yaml
  std::string config_file;
  if (argc >= 2) {
    config_file = argv[1];
  } else {
    config_file = ament_index_cpp::get_package_share_directory("robot_nico") +
      "/config/dummy_mission.yaml";
  }

  behavior_architecture::ActionConfig config;
  try {
    config = behavior_architecture::parse_config(config_file);
  } catch (const std::exception & e) {
    RCLCPP_ERROR(rclcpp::get_logger("main"), "Failed to parse config: %s", e.what());
    return 1;
  }

  auto blackboard = BT::Blackboard::create();
  auto node = std::make_shared<rclcpp::Node>(config.node_name);
  blackboard->set("node", node);
  behavior_architecture::setup_blackboard_from_config(blackboard, config);

  auto orchestrator = std::make_shared<robot_nico::RobotNicoOrchestrator>(blackboard);

  // on_configure creates and registers the behavior runners from blackboard config
  orchestrator->trigger_transition(lifecycle_msgs::msg::Transition::TRANSITION_CONFIGURE);
  orchestrator->trigger_transition(lifecycle_msgs::msg::Transition::TRANSITION_ACTIVATE);

  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(node);
  for (auto & runner : orchestrator->get_runners()) {
    executor.add_node(runner->get_node_base_interface());
  }
  executor.add_node(orchestrator->get_node_base_interface());

  RCLCPP_INFO(rclcpp::get_logger("main"), "Robot Nico starting...");

  executor.spin();

  rclcpp::shutdown();
  return 0;
}
