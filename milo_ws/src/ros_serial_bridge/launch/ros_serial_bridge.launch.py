from pathlib import Path
from ament_index_python import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


MILO_NAMESPACE = "milo"
PKG_NAME = "ros_serial_bridge"
PKG_PATH = get_package_share_directory(PKG_NAME)

def generate_launch_description():

    ros_serial_config = Path(PKG_PATH) / "config" / "ros_serial_config.yml"

    # Launch args
    launch_args = [
        DeclareLaunchArgument(
            "use_sim_time", 
            default_value="false", 
            description="Use simulation time"),

        DeclareLaunchArgument(
            "log_level",
            default_value="INFO",
            description="Log level: DEBUG, INFO, WARN, ERROR, FATAL"
        )
    ]

    ros_serial_bridge_node = Node(
        package="ros_serial_bridge",
        executable="ros_serial_bridge_node",
        name="ros_serial_bridge_node",
        namespace=MILO_NAMESPACE,
        output="screen",
        parameters=[
            ros_serial_config,
            {"use_sim_time": LaunchConfiguration("use_sim_time")}
        ],
        arguments=[
            "--ros-args",
            "--log-level", LaunchConfiguration("log_level")
        ]
    )

    return LaunchDescription(launch_args + [
        ros_serial_bridge_node,
    ])