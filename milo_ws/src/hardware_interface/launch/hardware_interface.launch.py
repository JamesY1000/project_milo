from pathlib import Path
from ament_index_python import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


MILO_NAMESPACE = "milo"
PKG_NAME = "hardware_interface"
PKG_PATH = get_package_share_directory(PKG_NAME)
SERIAL_DRIVE_PKG = "serial_driver"
SERIAL_DRIVE_PKG_PATH = Path(get_package_share_directory(SERIAL_DRIVE_PKG))

def generate_launch_description():

    hardware_interface_config = Path(PKG_PATH) / "config" / "hardware_interface_config.yml"

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

    hardware_interface_node = Node(
        package="hardware_interface",
        executable="hardware_interface_node",
        name="hardware_interface_node",
        namespace=MILO_NAMESPACE,
        output="screen",
        parameters=[
            hardware_interface_config,
            {"use_sim_time": LaunchConfiguration("use_sim_time")}
        ],
        arguments=[
            "--ros-args",
            "--log-level", LaunchConfiguration("log_level")
        ]
    )

    serial_driver_launch_ = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            str(SERIAL_DRIVE_PKG_PATH / "launch" / "serial_driver_bridge_node.launch.py")
        ]),
        launch_arguments={
            "params_file": str(hardware_interface_config)
        }.items()
    )

    return LaunchDescription(launch_args + [
        hardware_interface_node,
        serial_driver_launch_,
    ])