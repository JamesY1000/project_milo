from pathlib import Path
from ament_index_python import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource


MILO_CONTROL_PKG = "milo_control"
MILO_CONTROL_PKG_PATH = Path(get_package_share_directory(MILO_CONTROL_PKG))

HARDWARE_INTERFACE_PKG = "hardware_interface"
HARDWARE_INTERFACE_PKG_PATH = Path(get_package_share_directory(HARDWARE_INTERFACE_PKG))

def generate_launch_description():
    milo_control_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            str(MILO_CONTROL_PKG_PATH / "launch" / "milo_control.launch.py")
        ])
    )

    ros_serial_bridge_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            str(HARDWARE_INTERFACE_PKG_PATH / "launch" / "hardware_interface.launch.py")
        ])
    )

    return LaunchDescription([
        milo_control_launch,
        ros_serial_bridge_launch,
    ])