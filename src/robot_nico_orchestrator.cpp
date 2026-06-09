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

#include "robot_nico/robot_nico_orchestrator.hpp"
#include "behavior_architecture/behavior_config.hpp"
#include "behavior_architecture/behavior_runner.hpp"
#include "behavior_architecture/orchestrator_factory.hpp"
#include "robot_nico/bt_nodes/log_message_action.hpp"

namespace robot_nico
{

// Register this orchestrator with the factory
static behavior_architecture::OrchestratorRegistrar<RobotNicoOrchestrator> 
  robot_nico_registrar("robot_nico");

RobotNicoOrchestrator::RobotNicoOrchestrator(BT::Blackboard::Ptr blackboard)
: BaseOrchestrator("robot_nico_orchestrator", blackboard),
  state_(State::INIT)
{
  RCLCPP_INFO(get_logger(), "RobotNicoOrchestrator created");
}

rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
RobotNicoOrchestrator::on_configure(const rclcpp_lifecycle::State & state)
{
  if (runners_.empty()) {
    auto registrar = [](BT::BehaviorTreeFactory & factory) {
      factory.registerNodeType<bt_nodes::LogMessageAction>("LogMessage");
    };

    // Prefer config from blackboard (set by mission_executor or main)
    std::vector<behavior_architecture::BehaviorConfig> behaviors;
    std::vector<std::string> plugins;
    std::string pkg_name = "robot_nico";

    if (!blackboard_->get("behaviors_config", behaviors) ||
      !blackboard_->get("plugin_libraries", plugins))
    {
      // Standalone fallback — hardcoded defaults
      behaviors = {
        {"idle_runner", "behaviors/dummy_idle.xml", "robot_nico", 50},
        {"guide_runner", "behaviors/dummy_guide.xml", "robot_nico", 50},
        {"recognise_runner", "behaviors/dummy_recognise.xml", "robot_nico", 50},
        {"search_runner", "behaviors/dummy_search.xml", "robot_nico", 50},
        {"cross_runner", "behaviors/dummy_cross.xml", "robot_nico", 50},
      };
    } else {
      std::ignore = blackboard_->get("package_name", pkg_name);
    }

    for (auto & bc : behaviors) {
      auto runner = std::make_shared<behavior_architecture::BehaviorRunner>(
        blackboard_, bc.name, bc.behavior_file, plugins,
        bc.package_name.empty() ? pkg_name : bc.package_name,
        bc.control_period_ms, registrar);
      register_runner(bc.name, runner);
    }
  }
  return BaseOrchestrator::on_configure(state);
}

void RobotNicoOrchestrator::control_cycle()
{
  switch (state_) {
    case State::INIT:
      RCLCPP_INFO(get_logger(), "State: INIT");
      go_to_state(static_cast<int>(State::IDLE));
      break;

    case State::IDLE:
      if (check_behavior_finished()) {
        RCLCPP_INFO(get_logger(), "State 1 behavior finished with status: %s", 
          last_status_.c_str());
        if (last_status_ == "SUCCESS") {
          std::string action;

          if (blackboard_->get("action", action)) {
            RCLCPP_INFO(get_logger(), "Detected action: %s", action.c_str());

            if (action == "guiado") {
              go_to_state(static_cast<int>(State::GUIDE));
            } else if (action == "reconocimiento") {
              go_to_state(static_cast<int>(State::RECOGNISE));
            } else if (action == "busqueda") {
              go_to_state(static_cast<int>(State::SEARCH));
            } else {
              RCLCPP_WARN(get_logger(), "Unknown action: %s", action.c_str());
              go_to_state(static_cast<int>(State::IDLE));
            }
          } else {
            RCLCPP_WARN(get_logger(), "No action extracted");
            go_to_state(static_cast<int>(State::IDLE));
          }

        } else {
          RCLCPP_WARN(get_logger(), "State 1 failed, retrying...");
          go_to_state(static_cast<int>(State::IDLE));
        }
      }
      break;

    case State::GUIDE:
      if (check_behavior_finished()) {
        RCLCPP_INFO(get_logger(), "State 2 behavior finished with status: %s", 
          last_status_.c_str());
        if (last_status_ == "SUCCESS") {
          go_to_state(static_cast<int>(State::STOP));
        } else {
          RCLCPP_WARN(get_logger(), "State 2 failed, retrying...");
          go_to_state(static_cast<int>(State::GUIDE));
        }
      }
      break;
    case State::CROSS:
      if (check_behavior_finished()) {
        RCLCPP_INFO(get_logger(), "State 2 behavior finished with status: %s", 
          last_status_.c_str());
        if (last_status_ == "SUCCESS") {
          go_to_state(static_cast<int>(State::STOP));
        } else {
          RCLCPP_WARN(get_logger(), "State 2 failed, retrying...");
          go_to_state(static_cast<int>(State::CROSS));
        }
      }
      break;

    case State::RECOGNISE:
      if (check_behavior_finished()) {
        RCLCPP_INFO(get_logger(), "State 2 behavior finished with status: %s", 
          last_status_.c_str());
        if (last_status_ == "SUCCESS") {
          go_to_state(static_cast<int>(State::STOP));
        } else {
          RCLCPP_WARN(get_logger(), "State 2 failed, retrying...");
          go_to_state(static_cast<int>(State::RECOGNISE));
        }
      }
      break;
    case State::SEARCH:
      if (check_behavior_finished()) {
        RCLCPP_INFO(get_logger(), "State 2 behavior finished with status: %s", 
          last_status_.c_str());
        if (last_status_ == "SUCCESS") {
          go_to_state(static_cast<int>(State::STOP));
        } else {
          RCLCPP_WARN(get_logger(), "State 2 failed, retrying...");
          go_to_state(static_cast<int>(State::SEARCH));
        }
      }
      break;
    case State::STOP:
      RCLCPP_INFO_ONCE(get_logger(), "State: STOP - All behaviors completed");
      break;
  }
}

void RobotNicoOrchestrator::go_to_state(int state)
{
  state_ = static_cast<State>(state);

  switch (state_) {
    case State::INIT:
      RCLCPP_INFO(get_logger(), "Transitioning to INIT");
      deactivate_all_runners();
      break;

    case State::IDLE:
      RCLCPP_INFO(get_logger(), "Transitioning to IDLE");
      deactivate_all_runners();
      activate_runner("idle_runner");
      break;

    case State::GUIDE:
      RCLCPP_INFO(get_logger(), "Transitioning to GUIDE");
      deactivate_all_runners();
      activate_runner("guide_runner");
      break;
    case State::CROSS:
      RCLCPP_INFO(get_logger(), "Transitioning to CROSS");
      deactivate_all_runners();
      activate_runner("cross_runner");
      break;

    case State::RECOGNISE:
      RCLCPP_INFO(get_logger(), "Transitioning to RECOGNISE");
      deactivate_all_runners();
      activate_runner("recognise_runner");
      break;

    case State::SEARCH:
      RCLCPP_INFO(get_logger(), "Transitioning to SEARCH");
      deactivate_all_runners();
      activate_runner("search_runner");
      break;

    case State::STOP:
      RCLCPP_INFO(get_logger(), "Transitioning to STOP");
      deactivate_all_runners();
      break;
  }
}

}  // namespace robot_nico
