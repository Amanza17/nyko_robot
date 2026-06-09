#!/usr/bin/env python3

from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    """Launch local HRI services used by robot_nico BT nodes."""

    tts_node = Node(
        package='simple_hri',
        executable='tts_service_local',
        name='tts_service_local',
        output='screen',
        emulate_tty=True,
    )

    soundplay_node = Node(
        package='sound_play',
        executable='soundplay_node.py',
        name='soundplay_node',
        output='screen',
        emulate_tty=True,
    )

    stt_node = Node(
        package='simple_hri',
        executable='stt_service_local',
        name='stt_service_local',
        output='screen',
        emulate_tty=True,
    )

    extract_hugg_node = Node(
        package='simple_hri',
        executable='extract_service_hugg',
        name='extract_service_hugg',
        output='screen',
        emulate_tty=True,
    )
    analyze_image_hugg_node = Node(
        package='simple_hri',
        executable='analyze_image_service',
        name='analyze_image_service',
        output='screen',
        emulate_tty=True,
    )
    return LaunchDescription([
        tts_node,
        soundplay_node,
        stt_node,
        extract_hugg_node,
        analyze_image_hugg_node,

    ])
