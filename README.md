# rosbag_to_pcd_ws

这个工作空间用于从 ROS 2 rosbag2 数据中导出离线数据产物。目前包含两个独立的小工具包：

| 包 | 作用 |
| --- | --- |
| `rosbag_pointcloud_to_pcd` | 从 rosbag2 中读取一个或多个 `sensor_msgs/msg/PointCloud2` 话题，合并后导出为一个 PCD 文件。 |
| `rosbag_odom_to_waypoints` | 从 rosbag2 中读取 `nav_msgs/msg/Odometry`，导出 waypoint YAML。 |

## 目录结构

```text
src/
  rosbag_pointcloud_to_pcd/
    launch/pointcloud_to_pcd.launch.py
    src/pointcloud_to_pcd_node.cpp

  rosbag_odom_to_waypoints/
    examples/bags/bag_p_2026-06-11-10-17-37/
    launch/odom_to_waypoints.launch.py
    src/odom_to_waypoints_node.cpp
```

`rosbag_odom_to_waypoints` 包内带有一个示例 rosbag2 数据，话题为 `/mavros/odometry/out`。该示例 bag 不包含 `PointCloud2` 话题，因此不能作为 `rosbag_pointcloud_to_pcd` 的输入示例。

## 编译

参考 `.vscode/tasks.json`，工作区默认使用 symlink install，并导出 compile commands：

```bash
source /opt/ros/humble/setup.bash
colcon build --symlink-install --event-handlers console_direct+ --cmake-args -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

编译后加载环境：

```bash
source install/setup.bash
```

## 输出目录

默认输出统一放在工作区根目录的 `out/` 下：

```text
out/
  pcd/
    output.pcd
  waypoints/
    waypoints.yaml
```

## 使用示例

导出示例 bag 中的里程计轨迹：

```bash
ros2 launch rosbag_odom_to_waypoints odom_to_waypoints.launch.py
```

指定点云 bag 并导出 PCD：

```bash
ros2 launch rosbag_pointcloud_to_pcd pointcloud_to_pcd.launch.py \
  bag_path:=/path/to/pointcloud_bag \
  topic_name:=/points \
  pcd_output_path:=out/pcd/output.pcd
```

更多参数说明见各包 README。
