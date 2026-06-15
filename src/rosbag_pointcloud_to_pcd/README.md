# rosbag_pointcloud_to_pcd

`rosbag_pointcloud_to_pcd` 用于从 ROS 2 rosbag2 数据中读取 `sensor_msgs/msg/PointCloud2` 话题，将所有匹配话题的点云消息累加后导出为一个二进制 PCD 文件。

## 节点

| 节点 | 说明 |
| --- | --- |
| `pointcloud_to_pcd_node` | 读取 rosbag2 中的点云消息，并保存为 PCD。 |

## 参数

| 参数 | 默认值 | 说明 |
| --- | --- | --- |
| `bag_path` | 空字符串 | rosbag2 数据目录路径。必须提供。 |
| `topic_name` | `/points` | rosbag2 中的 `PointCloud2` 话题名。 |
| `pcd_output_path` | 工作区 `out/pcd/output.pcd` | 输出 PCD 文件路径。 |

当前节点使用 `sqlite3` 存储后端读取 rosbag2。

## 使用

```bash
source install/setup.bash
ros2 launch rosbag_pointcloud_to_pcd pointcloud_to_pcd.launch.py \
  bag_path:=/path/to/pointcloud_bag \
  topic_name:=/points \
  pcd_output_path:=out/pcd/output.pcd
```

也可以直接运行节点：

```bash
ros2 run rosbag_pointcloud_to_pcd pointcloud_to_pcd_node \
  --ros-args \
  -p bag_path:=/path/to/pointcloud_bag \
  -p topic_name:=/points \
  -p pcd_output_path:=out/pcd/output.pcd
```

## 输入要求

- `bag_path` 指向 rosbag2 数据目录，而不是单个 `.db3` 文件。
- `topic_name` 对应的话题类型必须是 `sensor_msgs/msg/PointCloud2`。
- 节点会自动创建输出文件的父目录。

## 说明

工作区内当前自带的示例 bag 只有里程计、IMU、MAVROS 状态等话题，不包含 `PointCloud2`，所以没有作为本包默认输入。
