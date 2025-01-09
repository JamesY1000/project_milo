from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch_ros.actions import Node
from launch.substitutions import PathJoinSubstitution
from ament_index_python.packages import get_package_share_directory

import os

def generate_launch_description():

    milo_localisation_pkg = "milo_localisation"
    
    milo_localisation_launch = IncludeLaunchDescription(
        PathJoinSubstitution([
            get_package_share_directory(milo_localisation_pkg),
            "launch",
            "milo_localisation.launch.py"
        ])

    
    # TODO: ADD milo_mapping package

    # milo_mapping_launch = ...


    )
    
    return LaunchDescription([
        milo_localisation_launch
    ])