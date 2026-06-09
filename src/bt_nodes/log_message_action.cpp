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

#include "robot_nico/bt_nodes/log_message_action.hpp"

namespace robot_nico
{
namespace bt_nodes
{

LogMessageAction::LogMessageAction(
  const std::string & name,
  const BT::NodeConfig & config)
: BT::SyncActionNode(name, config)
{
  // Get the ROS node from blackboard
  config.blackboard->get("node", node_);
}

BT::PortsList LogMessageAction::providedPorts()
{
  return {
    BT::InputPort<std::string>("message", "Message to log")
  };
}

BT::NodeStatus LogMessageAction::tick()
{
  // Get the message from the input port
  auto message = getInput<std::string>("message");
  
  if (!message) {
    RCLCPP_ERROR(
      node_->get_logger(),
      "LogMessageAction: Missing required input [message]");
    return BT::NodeStatus::FAILURE;
  }

  // Log the message
  RCLCPP_INFO(
    node_->get_logger(),
    "LogMessageAction: %s", message.value().c_str());

  return BT::NodeStatus::SUCCESS;
}

}  // namespace bt_nodes
}  // namespace robot_nico
