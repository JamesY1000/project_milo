from pathlib import Path

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.substitutions import LaunchConfiguration, Command
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import Node

def generate_launch_description():

    use_sim_time = LaunchConfiguration('use_sim_time')

    pkg_path = Path(get_package_share_directory('milo_description')).as_posix()
    relative_path = Path('models', 'milo_bot', 'robot.urdf.xacro').as_posix()
    xacro_file = Path(pkg_path, relative_path).as_posix()

    robot_description_config = Command(
        ['xacro ', xacro_file]
    )

    params = {
        'robot_description': robot_description_config, 
        'use_sim_time': use_sim_time
    }

    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[params]
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'use_sim_time',
            default_value='false',
            description='Use sim time if true'),

        robot_state_publisher
    ])