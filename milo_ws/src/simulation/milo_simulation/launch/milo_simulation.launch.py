import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import PathJoinSubstitution

def generate_launch_description():

    milo_description_pkg = "milo_description"
    milo_simulation_pkg = "milo_simulation" # TODO: Change to local path?
    gazebo_world = os.path.join(get_package_share_directory(milo_description_pkg), "worlds", "obstacles.world")
    gazebo_params_path = os.path.join(get_package_share_directory(milo_simulation_pkg), "config", "gazebo_params.yaml")
    rviz_config_path = os.path.join(get_package_share_directory(milo_simulation_pkg), "config", "view_milo.rviz")

    milo_description_launch = IncludeLaunchDescription(
            PathJoinSubstitution([
            get_package_share_directory(milo_description_pkg),
            "launch",
            "milo_description.launch.py"
            ])
        )

    # TODO: Separate gazebo into client and server
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory(
                    'gazebo_ros'), 
                    'launch', 
                    'gazebo.launch.py')),
            launch_arguments={
                'world': gazebo_world,
                'extra_gazebo_args': '--ros-args --params-file ' + gazebo_params_path
                }.items(),
    )

    gazebo_server = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory("gazebo_ros"),
                "launch",
                "gzserver.launch.py"
            )
        ),
        launch_arguments={
            "world": gazebo_world,
            "extra_gazebo_args": "--ros-args --params-file " + gazebo_params_path
        }.items(),
    )

    gazebo_client = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory("gazebo_ros"),
                "launch",
                "gzclient.launch.py"
            )
        )
    )

    # Run spawner node from gazebo_ros package. Entity name doesn't matter
    spawn_entity = Node(
        package='gazebo_ros', 
        executable='spawn_entity.py', 
        arguments=['-topic', 
                   'robot_description',
                   '-entity', 
                   'milo'],
        output='screen')

    rviz2 = Node(
        package='rviz2',
        namespace='',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_config_path],
    )
    
    return LaunchDescription([
        milo_description_launch,
        rviz2,
        # gazebo,
        gazebo_server,
        gazebo_client,
        spawn_entity
        
    ])