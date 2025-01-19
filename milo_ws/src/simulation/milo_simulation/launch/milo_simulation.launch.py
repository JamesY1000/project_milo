from pathlib import Path

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import PathJoinSubstitution

def generate_launch_description():

    milo_description_pkg = "milo_description"
    milo_slam_pkg = "milo_slam"

    gazebo_world = (Path(__file__).resolve().parent.parent
                    / "worlds"
                    / "obstacles.world"
                    ).as_posix()

    gazebo_params_path = (Path(__file__).resolve().parent.parent 
                          / "config" 
                          / "gazebo_params.yml"
                    ).as_posix()
    
    rviz_config_path = (Path(__file__).resolve().parent.parent 
                        / "config" 
                        / "view_milo.rviz"
                        ).as_posix()

    milo_description_launch = IncludeLaunchDescription(
            PathJoinSubstitution([
            get_package_share_directory(milo_description_pkg),
            "launch",
            "milo_description.launch.py"
            ])
        )
    
    # milo_slam = IncludeLaunchDescription(
    #     PathJoinSubstitution([
    #         get_package_share_directory(milo_slam_pkg),
    #         "launch",
    #         "milo_slam.launch.py"
    #     ])
    # )

    gazebo_server = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            Path(
                get_package_share_directory("gazebo_ros"),
                "launch",
                "gzserver.launch.py"
            ).as_posix()
        ),
        launch_arguments={
            "world": gazebo_world,
            "extra_gazebo_args": "--ros-args --params-file " + gazebo_params_path
        }.items(),
    )

    gazebo_client = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            Path(
                get_package_share_directory("gazebo_ros"),
                "launch",
                "gzclient.launch.py"
            ).as_posix()
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
        # milo_slam, # Include in bringup, not simulation
        rviz2,
        gazebo_server,
        gazebo_client,
        spawn_entity
    ])