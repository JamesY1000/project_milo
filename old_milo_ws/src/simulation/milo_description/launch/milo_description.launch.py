import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node


def generate_launch_description():

    package_name = 'milo_description'

    rsp = IncludeLaunchDescription(PythonLaunchDescriptionSource([os.path.join(
        get_package_share_directory(package_name), 'launch', 'rsp.launch.py'
        )]), 
        launch_arguments={'use_sim_time': 'true', 'use_ros2_control': 'true'}.items()
        )

    diff_drive_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["diff_drive_controller"],
    )

    joint_broad_spawner=Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_broad"],
    )

    return LaunchDescription([
        rsp,
        diff_drive_spawner,
        joint_broad_spawner,
    ])


