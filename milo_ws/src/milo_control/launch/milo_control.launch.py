from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


MILO_NAMESPACE = "milo"

def generate_launch_description():

    # Launch args
    launch_args = [
        DeclareLaunchArgument("use_sim_time", default_value="false", description="Use simulation time"),
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

    teleop_twist_joy_node = Node(
        package="teleop_twist_joy",
        executable="teleop_node",
        name="teleop_joy_node",
        namespace=MILO_NAMESPACE,
        output="screen",
        parameters=[{
            "use_sim_time": LaunchConfiguration("use_sim_time"),
            "require_enable_button": False,
            # Add your button/axis mappings here (use config file later on)
            # 'axis_linear.x': 1,
            # 'axis_angular.yaw': 0,
            # 'enable_button': 0,
            # 'scale_linear.x': 0.5,
            # 'scale_angular.yaw': 1.0,
        }],
    )

    # cmd_vel -> serial bridge? 




    return LaunchDescription(launch_args + [
        joystick_node,
        teleop_twist_joy_node,
    ])