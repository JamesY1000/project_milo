import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource

from launch_ros.actions import Node


def generate_launch_description():

    # Include rsp launch file, set sim time=true
    package_name = 'milo_description'
    gazebo_world = os.path.join(get_package_share_directory(package_name), 'worlds', 'obstacles.world')
    gazebo_params_path = os.path.join(get_package_share_directory(package_name), 'config', 'gazebo_params.yaml')


    # Change use_ros2_control to true when you're using ros2 with wheel encoders
    rsp = IncludeLaunchDescription(PythonLaunchDescriptionSource([os.path.join(
        get_package_share_directory(package_name), 'launch', 'rsp.launch.py'
        )]), 
        # launch_arguments={'use_sim_time': 'true', 'use_ros2_control': 'true'}.items()
        launch_arguments={'use_sim_time': 'true', 'use_ros2_control': 'false'}.items()
        )

    
    # Include gazebo launch file, provided by gazebo_ros package
    gazebo = IncludeLaunchDescription(PythonLaunchDescriptionSource([os.path.join(
        get_package_share_directory('gazebo_ros'), 'launch', 'gazebo.launch.py')]),
        launch_arguments={
            'world': gazebo_world,
            'extra_gazebo_args': '--ros-args --params-file ' + gazebo_params_path
        }.items(),
    )
    

    # Run spawner node from gazebo_ros package. Entity name doesn't matter
    spawn_entity = Node(package='gazebo_ros', executable='spawn_entity.py', 
                        arguments=['-topic', 'robot_description',
                                   '-entity', 'milo'],
                                   output='screen')
    

    
    # Open rviz
    rviz2 = Node(
        package='rviz2',
        namespace='',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', os.path.join(get_package_share_directory('milo_description'), 'config', 'view_milo.rviz')],
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


    # Launch them all!
    # Uncomment diff_drive and joint_broad when you're using ros2_control with wheel encoders
    return LaunchDescription([
        rsp,
        gazebo,
        spawn_entity,
        rviz2,
        # diff_drive_spawner,
        # joint_broad_spawner,
    ])


