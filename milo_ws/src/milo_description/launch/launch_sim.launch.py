import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource

from launch_ros.actions import Node


def generate_launch_description():

    # Include rsp launch file, set sim time=true
    package_name = 'milo_description'
    gazebo_world = os.path.join(get_package_share_directory(package_name), 'worlds', 'obstacles.world')

    rsp = IncludeLaunchDescription(PythonLaunchDescriptionSource([os.path.join(
                get_package_share_directory(package_name), 'launch', 'rsp.launch.py'
                                )]), launch_arguments={'use_sim_time': 'true'}.items()
            )

    
    # Include gazebo launch file, provided by gazebo_ros package
    gazebo = IncludeLaunchDescription(PythonLaunchDescriptionSource([os.path.join(
        get_package_share_directory('gazebo_ros'), 'launch', 'gazebo.launch.py')]),
        launch_arguments={'world': gazebo_world}.items()
            )
    

    # Run spawner node from gazebo_ros package. Entity name doesn't matter
    spawn_entity = Node(package='gazebo_ros', executable='spawn_entity.py', 
                        arguments=['-topic', 'robot_description',
                                   '-entity', 'my_bot'],
                                   output='screen')
    
    # Open rviz
    rviz2 = Node(
        package='rviz2',
        namespace='',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', os.path.join(get_package_share_directory('milo_description'), 'config', 'view_milo.rviz')],
    )
    
    # Launch them all!
    return LaunchDescription([
        rsp,
        gazebo,
        spawn_entity,
        rviz2,
    ])


