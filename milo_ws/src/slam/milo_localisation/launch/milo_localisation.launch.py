from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration
from ament_index_python.packages import get_package_share_directory

import os

def generate_launch_description():

    ekf_config = os.path.join(
            os.path.dirname(__file__),
            "..",
            "config",
            "ekf.yaml"
        )

    robot_localisation_node = Node(
        package="robot_localization",
        executable="ekf_node",
        name="ekf_filter_node",
        output="screen",
        parameters=[ekf_config, {'use_sim_time': True}] # TODO: Change this to be more elegant - pass use_sim_time : true or something as an argument/param from milo_bringup
        )
    
    return LaunchDescription([
        robot_localisation_node
    ])