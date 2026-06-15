from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument(
            'bag',
            default_value=PathJoinSubstitution([
                FindPackageShare('rosbag_odom_to_waypoints'),
                'examples',
                'bags',
                'bag_p_2026-06-11-10-17-37'
            ]),
            description='rosbag2 数据目录路径；默认使用包内示例 bag'
        ),
        DeclareLaunchArgument(
            'topic',
            default_value='/mavros/odometry/out',
            description='rosbag2 中的 Odometry 话题名'
        ),
        DeclareLaunchArgument(
            'out',
            default_value=PathJoinSubstitution([
                FindPackageShare('rosbag_odom_to_waypoints'),
                '..',
                '..',
                '..',
                '..',
                'out',
                'waypoints',
                'waypoints.yaml'
            ]),
            description='导出的 waypoint YAML 路径；默认写入工作区 out/waypoints'
        ),
        DeclareLaunchArgument(
            'frame',
            default_value='map',
            description='输出 YAML 的 frame_id'
        ),
        DeclareLaunchArgument(
            'storage',
            default_value='sqlite3',
            description='rosbag2 存储后端，例如 sqlite3 或 mcap'
        ),

        Node(
            package='rosbag_odom_to_waypoints',
            executable='odom_to_waypoints_node',
            name='odom_to_waypoints_node',
            output='screen',
            parameters=[{
                'bag': LaunchConfiguration('bag'),
                'topic': LaunchConfiguration('topic'),
                'out': LaunchConfiguration('out'),
                'frame': LaunchConfiguration('frame'),
                'storage': LaunchConfiguration('storage')
            }]
        )
    ])
