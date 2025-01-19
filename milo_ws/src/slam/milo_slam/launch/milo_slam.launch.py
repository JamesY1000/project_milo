from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch_ros.actions import Node
from launch.substitutions import PathJoinSubstitution, LaunchConfiguration
from ament_index_python.packages import get_package_share_directory
from pathlib import Path

def generate_launch_description():

    remappings = [
            ("/rgb/image", "/camera/image_raw"),
            ("/rgb/camera_info", "/camera/camera_info"),
            ("/scan", "/milo/scan"),
            ("/odom", "/odometry/filtered")
    ]

    milo_localisation_pkg = "milo_localisation"
    milo_slam_pkg = "milo_slam"
    rtab_map_rgb_ekf_lidar_config = Path(get_package_share_directory(milo_slam_pkg), "config", "rtabmap_rgb_ekf_lidar.yml")
    rtab_map_ekf_lidar_config = Path(get_package_share_directory(milo_slam_pkg), "config", "rtabmap_ekf_lidar.yml")


    milo_localisation_launch = IncludeLaunchDescription(
        PathJoinSubstitution([
            get_package_share_directory(milo_localisation_pkg),
            "launch",
            "milo_localisation.launch.py"
        ])
    )

    rtab_slam_map = Node(
        package="rtabmap_slam",
        executable="rtabmap",
        output="screen",
        parameters=[rtab_map_rgb_ekf_lidar_config], # Can be switched out for ekf_lidar mode depending on rpi performance
        remappings=remappings,
        arguments=['--delete_db_on_start'] # Deletes database, investigate when we want to reuse maps
    )

    rtab_map_viz = Node(
        package="rtabmap_viz",
        executable="rtabmap_viz",
        output="screen",
        parameters=[rtab_map_rgb_ekf_lidar_config],
        remappings=remappings
    )
    
    return LaunchDescription([
        milo_localisation_launch,
        rtab_slam_map,
        rtab_map_viz
    ])