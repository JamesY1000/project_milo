from pathlib import Path
from ament_index_python import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


MILO_NAMESPACE = "milo"
PKG_NAME = "milo_control"
PKG_PATH = get_package_share_directory(PKG_NAME)

def generate_launch_description():

    controller_teleop_config = Path(PKG_PATH) / "config" / "control_config.yml"

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

    joystick_node = Node(
        package="joy",
        executable="joy_node",
        name="joystick_node",
        namespace=MILO_NAMESPACE,
        output="screen",
        parameters=[{
            "use_sim_time": LaunchConfiguration("use_sim_time"),
        }]
    )

    controller_teleop_node = Node(
        package="milo_control",
        executable="controller_teleop_node",
        name="controller_teleop_node",
        namespace=MILO_NAMESPACE,
        output="screen",
        parameters=[
            controller_teleop_config,
            {"use_sim_time": LaunchConfiguration("use_sim_time")}
        ],
        arguments=[
            "--ros-args",
            "--log-level", LaunchConfiguration("log_level")
        ]
    )


    # Maybe necessary for simulation?

    # teleop_twist_joy_node = Node(
    #     package="teleop_twist_joy",
    #     executable="teleop_node",
    #     name="teleop_joy_node",
    #     namespace=MILO_NAMESPACE,
    #     output="screen",
    #     parameters=[{
    #         "use_sim_time": LaunchConfiguration("use_sim_time"),
    #         "require_enable_button": False,
    #         # Add your button/axis mappings here (use config file later on)
    #         # 'axis_linear.x': 1,
    #         # 'axis_angular.yaw': 0,
    #         # 'enable_button': 0,
    #         # 'scale_linear.x': 0.5,
    #         # 'scale_angular.yaw': 1.0,
    #     }],
    # )

    return LaunchDescription(launch_args + [
        joystick_node,
        controller_teleop_node,
    ])