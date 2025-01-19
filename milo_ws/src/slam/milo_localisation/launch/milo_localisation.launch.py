from launch import LaunchDescription
from launch_ros.actions import Node
from pathlib import Path

def generate_launch_description():

    ekf_config = (Path(__file__).resolve().parent.parent
                  / "config"
                  / "ekf.yml"
                  )

    robot_localisation_node = Node(
        package="robot_localization",
        executable="ekf_node",
        name="ekf_filter_node",
        output="screen",
        parameters=[ekf_config, {'use_sim_time': True}] # TODO: Change this to be more elegant - pass use_sim_time : true or something as an argument/param from milo_bringup
        )
    
    # odom_to_tf = Node(
    #     package="milo_localisation",
    #     executable="odom_to_tf.py",
    #     name="odom_to_tf_node",
    #     output="screen",
    #     )    
    return LaunchDescription([
        robot_localisation_node,
        # odom_to_tf
    ])