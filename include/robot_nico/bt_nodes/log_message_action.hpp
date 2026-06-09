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

#ifndef DUMMY_ROBOT__BT_NODES__LOG_MESSAGE_ACTION_HPP_
#define DUMMY_ROBOT__BT_NODES__LOG_MESSAGE_ACTION_HPP_

#include "behaviortree_cpp/action_node.h"
#include "rclcpp/rclcpp.hpp"

namespace robot_nico
{
namespace bt_nodes
{

/**
 * @brief Custom BT action node that logs a message
 * 
 * This is a simple example of how to create custom BT nodes for your robot.
 * It demonstrates:
 * - Getting parameters from XML (message port)
 * - Accessing ROS node from blackboard
 * - Using ROS logging
 */
class LogMessageAction : public BT::SyncActionNode
{
public:
  /**
   * @brief Constructor
   * @param name Node name
   * @param config Node configuration
   */
  LogMessageAction(const std::string & name, const BT::NodeConfig & config);

  /**
   * @brief Define the ports (inputs/outputs) for this node
   */
  static BT::PortsList providedPorts();

  /**
   * @brief Execute the action - logs the message
   */
  BT::NodeStatus tick() override;

private:
  rclcpp::Node::SharedPtr node_;
};

}  // namespace bt_nodes
}  // namespace robot_nico

#endif  // DUMMY_ROBOT__BT_NODES__LOG_MESSAGE_ACTION_HPP_
