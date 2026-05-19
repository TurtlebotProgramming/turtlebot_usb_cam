from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    params = os.path.join(
        get_package_share_directory('turtlebot_usb_cam'),
        'config', 'camera_params.yaml'
    )
    return LaunchDescription([
        Node(
            package='turtlebot_usb_cam',
            executable='usb_cam_node',
            name='usb_cam_node',
            parameters=[params],
            output='screen',
        )
    ])
