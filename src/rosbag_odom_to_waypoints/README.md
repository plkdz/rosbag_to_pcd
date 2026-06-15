# rosbag_odom_to_waypoints

`rosbag_odom_to_waypoints` 用于从 ROS 2 rosbag2 数据中读取 `nav_msgs/msg/Odometry` 话题，并导出 waypoint YAML 文件。

## 节点

| 节点 | 说明 |
| --- | --- |
| `odom_to_waypoints_node` | 读取 rosbag2 中的里程计位置，并写出 waypoint YAML。 |

## 参数

| 参数 | 默认值 | 说明 |
| --- | --- | --- |
| `bag` | 包内示例 bag | rosbag2 数据目录路径。 |
| `topic` | `/mavros/odometry/out` | rosbag2 中的 `Odometry` 话题名。 |
| `out` | 工作区 `out/waypoints/waypoints.yaml` | 输出 waypoint YAML 路径。 |
| `frame` | `map` | 输出 YAML 中的 `frame_id`。 |
| `storage` | `sqlite3` | rosbag2 存储后端，例如 `sqlite3` 或 `mcap`。 |

## 示例数据

包内带有一个示例 rosbag2：

```text
examples/bags/bag_p_2026-06-11-10-17-37/
  metadata.yaml
  bag_p_2026-06-11-10-17-37_0.db3
```

该 bag 中可用于本包的里程计话题是：

```text
/mavros/odometry/out
```

## 使用

使用默认示例 bag：

```bash
source install/setup.bash
ros2 launch rosbag_odom_to_waypoints odom_to_waypoints.launch.py
```

指定自己的 bag：

```bash
ros2 launch rosbag_odom_to_waypoints odom_to_waypoints.launch.py \
  bag:=/path/to/odom_bag \
  topic:=/odom \
  out:=out/waypoints/waypoints.yaml \
  frame:=map \
  storage:=sqlite3
```

也可以直接运行节点：

```bash
ros2 run rosbag_odom_to_waypoints odom_to_waypoints_node \
  --ros-args \
  -p bag:=/path/to/odom_bag \
  -p topic:=/odom \
  -p out:=out/waypoints/waypoints.yaml \
  -p frame:=map \
  -p storage:=sqlite3
```

## 输出格式

输出 YAML 结构如下：

```yaml
frame_id: map
waypoints:
  - point:
      x: 0.0
      y: 0.0
      z: 0.0
```

每条 waypoint 对应一帧 `nav_msgs/msg/Odometry` 的 `pose.pose.position`。
