from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument(
            'bag_path',
            default_value='/tmp/example_bag',
            description='rosbag2 数据目录路径'
        ),
        DeclareLaunchArgument(
            'topic_names_csv',
            default_value='/lidar/front/points,/lidar/rear/points,/points',
            description='多个 PointCloud2 话题名，使用逗号分隔'
        ),
        DeclareLaunchArgument(
            'pcd_output_path',
            default_value=PathJoinSubstitution([
                FindPackageShare('rosbag_pointcloud_to_pcd'),
                '..',
                '..',
                '..',
                '..',
                'out',
                'pcd',
                'fused_output.pcd'
            ]),
            description='融合后的 PCD 输出路径；默认写入工作区 out/pcd'
        ),
        Node(
            package='rosbag_pointcloud_to_pcd',
            executable='pointcloud_to_pcd_node',
            name='pointcloud_to_pcd_multi_topics_node',
            parameters=[
                {'bag_path': LaunchConfiguration('bag_path')},
                {'topic_names_csv': LaunchConfiguration('topic_names_csv')},
                {'pcd_output_path': LaunchConfiguration('pcd_output_path')}
            ],
            output='screen'
        )
    ])
