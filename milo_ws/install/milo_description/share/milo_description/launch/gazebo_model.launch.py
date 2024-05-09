import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource

from launch_ros.actions import Node
import xacro


def generate_launch_description():

    # Must match name of robot in xacro file
    robotXacroName ='milo' #try robot?

    namePackage = 'milo_description'

    modelFileRelativePath = 'models/milo_bot/new_robot_core.xacro'

    worldFileRelativePath = 'worlds/obstacles.world'

    pathModelFile = os.path.join(get_package_share_directory(namePackage), modelFileRelativePath)

    pathWorldFile = os.path.join(get_package_share_directory(namePackage), worldFileRelativePath)

    robotDescription = xacro.process_file(pathModelFile).toxml()

    

    gazebo_rosPackageLaunch = PythonLaunchDescriptionSource(os.path.join(get_package_share_directory('gazebo_ros'), 'launch', 'gazebo.launch.py'))


    gazeboLaunch = IncludeLaunchDescription((gazebo_rosPackageLaunch), launch_arguments={'world': pathWorldFile}.items())

    
    # Gazebo ros node
    spawnModelNode = Node(package='gazebo_ros', executable='spawn_entity.py', 
                          arguments=['-topic', 'robot_description', '-entity', robotXacroName], output='screen')
    

    # Robot state publisher node
    nodeRobotStatePublisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[{'robot_description': robotDescription,
                     'use_sim_time': True}]
    )


    # Create empty launch description object
    launchDescriptionObject = LaunchDescription()

    # Add gazeboLaunch
    launchDescriptionObject.add_action(gazeboLaunch)


    # Add two nodes
    launchDescriptionObject.add_action(spawnModelNode)
    launchDescriptionObject.add_action(nodeRobotStatePublisher)


    return launchDescriptionObject
