#!/usr/bin/env python3
# Copyright 2025 Your Name
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os

import yaml
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction, TimerAction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def launch_setup(context, *args, **kwargs):
    """Build the node list from the unified YAML config.

    Nodes launched:
      1. llm_planner_node    — breaks the goal into steps (optional)
      2. llm_bt_agent_node   — generates BT XML per step (optional)
      3. llm_plan_executor   — runs the planning + execution FSM
      4. start_mission      — sends the mission after a short delay

    All LLM provider settings, BT parameters, goal, and context come from
    the config file so there is a single place to change them.

    Set launch_llm_nodes:=true to also start llm_planner and llm_bt_builder
    from this launch file. Leave it as false when those nodes are already
    running in another terminal.
    """
    mission_file = LaunchConfiguration('mission_file').perform(context)
    skills_file = LaunchConfiguration('skills_file').perform(context)
    launch_llm_nodes = (
        LaunchConfiguration('launch_llm_nodes').perform(context).lower() == 'true'
    )
    save_exec_arg = LaunchConfiguration('save_exec').perform(context).lower() == 'true'

    # If it's just a filename, assume it's in robot_nico/config
    if not os.path.isabs(mission_file):
        pkg_dir = get_package_share_directory('robot_nico')
        mission_file = os.path.join(pkg_dir, 'config', mission_file)

    with open(mission_file, 'r') as f:
        config = yaml.safe_load(f)

    # Allow save_exec to be defined either via launch CLI or inside the YAML
    save_exec = save_exec_arg or config.get('save_exec', False)

    # ── LLM provider fields ───────────────────────────────────────────────────
    planner_provider = config.get(
        'planner_llm_provider', config.get('llm_provider', 'gemini'))
    planner_model = config.get(
        'planner_llm_model', config.get('llm_model', 'gemini-2.5-flash'))
    planner_api_key = config.get(
        'planner_llm_api_key', config.get('llm_api_key', ''))

    bt_builder_provider = config.get(
        'bt_builder_llm_provider', config.get('llm_provider', 'gemini'))
    bt_builder_model = config.get(
        'bt_builder_llm_model', config.get('llm_model', 'gemini-2.5-flash'))
    bt_builder_api_key = config.get(
        'bt_builder_llm_api_key', config.get('llm_api_key', ''))

    # ── BT executor fields ────────────────────────────────────────────────────
    bt_nodes_package = config.get('bt_nodes_package', 'dummy_bt_nodes')
    bt_control_period_ms = config.get('bt_control_period_ms', 50)
    bt_timeout_sec = config.get('bt_timeout_sec', 60.0)

    plugin_libraries = config.get('plugin_libraries', [])
    plugin_library = plugin_libraries[0] if plugin_libraries else 'libdummy_bt_nodes_plugin.so'

    nodes = []

    # ── 1. LLM Planner node (optional) ───────────────────────────────────────
    if launch_llm_nodes:
        nodes.append(Node(
            package='llm_planner',
            executable='llm_planner_node',
            name='llm_planner_node',
            output='screen',
            emulate_tty=True,
            parameters=[{
                'llm_provider': planner_provider,
                'llm_model_id': planner_model,
                'llm_api_key':  planner_api_key,
            }],
        ))

    # ── 2. LLM BT builder node (optional) ────────────────────────────────────
    if launch_llm_nodes:
        nodes.append(Node(
            package='llm_bt_builder',
            executable='bt_rag_agent_node.py',
            name='llm_bt_agent',
            output='screen',
            emulate_tty=True,
            parameters=[{
                'llm_provider': bt_builder_provider,
                'llm_model_id': bt_builder_model,
                'llm_api_key':  bt_builder_api_key,
            }],
        ))

    # ── 3. Mission Executor (handles both fixed and LLM orchestrators) ─────────
    executor_args = [mission_file]
    if save_exec:
        executor_args.append('--save-exec')
    nodes.append(Node(
        package='behavior_architecture',
        executable='mission_executor',
        name='mission_executor',
        output='screen',
        emulate_tty=True,
        arguments=executor_args,
    ))

    # ── 4. Goal sender (reads goal + context from the same config file) ───────
    nodes.append(TimerAction(
        period=3.0,
        actions=[Node(
            package='behavior_architecture',
            executable='start_mission',
            name='start_mission',
            output='screen',
            emulate_tty=True,
            parameters=[{
                'mission_file': mission_file,
                'skills_file': skills_file,
            }],
        )],
    ))

    return nodes


def generate_launch_description():
    """Launch the robot_nico behavior system in LLM-guided mode.

    Nodes started:
      1. llm_planner_node    — generates step-by-step task plans via LLM    (optional)
      2. llm_bt_agent_node   — generates BT XML from step descriptions       (optional)
      3. llm_plan_executor   — orchestrates planning, BT generation, and execution
      4. start_mission      — sends the initial mission (after a 3 s delay)

    All configuration is loaded from a single YAML file (same unified format
    as dummy_mission.yaml for the fixed orchestrator, but with
    orchestrator_type: "llm").

    Set launch_llm_nodes:=false (default) when llm_planner and llm_bt_builder
    are already running.  Set it to true to start them from this launch file.

    Example:
      # LLM nodes already running externally:
      ros2 launch robot_nico robot_nico_llm.launch.py

      # Start everything from scratch:
      ros2 launch robot_nico robot_nico_llm.launch.py launch_llm_nodes:=true

      # Custom config:
      ros2 launch robot_nico robot_nico_llm.launch.py \\
        mission_file:=guide_mission.yaml launch_llm_nodes:=true
    """
    pkg_dir = get_package_share_directory('robot_nico')
    default_config = os.path.join(pkg_dir, 'config', 'waiter_mission_checking.yaml')
    default_skills = os.path.join(pkg_dir, 'config', 'robot_nico_skills.yaml')

    mission_file_arg = DeclareLaunchArgument(
        'mission_file',
        default_value=default_config,
        description=(
            'Path to the unified YAML mission file '
            '(orchestrator_type must be "llm"). Can be a filename in robot_nico/config.'
        ),
    )

    skills_file_arg = DeclareLaunchArgument(
        'skills_file',
        default_value=default_skills,
        description='Path to the YAML file listing the robot skills (skills: [...]).',
    )

    launch_llm_nodes_arg = DeclareLaunchArgument(
        'launch_llm_nodes',
        default_value='false',
        description=(
            'Whether to launch llm_planner and llm_bt_builder nodes here. '
            'Set to "true" to start the full pipeline from this launch file.'
        ),
    )

    save_exec_arg = DeclareLaunchArgument(
        'save_exec',
        default_value='false',
        description=(
            'Save generated BT XMLs to exec/<mission_name>/<timestamp>/. '
            'Set to "true" to enable execution logging.'
        ),
    )

    return LaunchDescription([
        mission_file_arg,
        skills_file_arg,
        launch_llm_nodes_arg,
        save_exec_arg,
        OpaqueFunction(function=launch_setup),
    ])
