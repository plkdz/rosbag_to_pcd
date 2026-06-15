#include <rclcpp/rclcpp.hpp>
#include <rosbag2_cpp/reader.hpp>
#include <rclcpp/serialization.hpp>
#include <rclcpp/serialized_message.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <filesystem>
#include <fstream>
#include <iomanip>

namespace fs = std::filesystem;

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rclcpp::Node>("odom_to_waypoints_node");

  // 声明参数
  node->declare_parameter<std::string>("bag", "");
  node->declare_parameter<std::string>("topic", "");
  node->declare_parameter<std::string>("out", "out/waypoints/waypoints.yaml");
  node->declare_parameter<std::string>("frame", "map");
  node->declare_parameter<std::string>("storage", "sqlite3"); // sqlite3 或 mcap

  // 获取参数
  std::string bag, topic, out, frame, storage;
  node->get_parameter("bag", bag);
  node->get_parameter("topic", topic);
  node->get_parameter("out", out);
  node->get_parameter("frame", frame);
  node->get_parameter("storage", storage);

  if (bag.empty() || topic.empty()) {
    RCLCPP_ERROR(node->get_logger(), "必须提供参数: bag 与 topic");
    return 1;
  }

  // 打开 bag
  rosbag2_cpp::Reader reader;
  rosbag2_storage::StorageOptions sopt;
  sopt.uri = bag;
  sopt.storage_id = storage;
  rosbag2_cpp::ConverterOptions copt{"cdr", "cdr"};
  reader.open(sopt, copt);

  rclcpp::Serialization<nav_msgs::msg::Odometry> ser;
  const fs::path output_path(out);
  if (output_path.has_parent_path()) {
    fs::create_directories(output_path.parent_path());
  }

  std::ofstream ofs(out);
  if (!ofs.is_open()) {
    RCLCPP_ERROR(node->get_logger(), "无法打开输出文件: %s", out.c_str());
    return 2;
  }

  // YAML 头
  ofs << "frame_id: " << frame << "\n";
  ofs << "waypoints:\n";
  ofs << std::setprecision(17);

  int count = 0;
  while (reader.has_next()) {
    auto msg = reader.read_next();
    if (msg->topic_name != topic) continue;

    rclcpp::SerializedMessage smsg(*msg->serialized_data);
    nav_msgs::msg::Odometry odom;
    ser.deserialize_message(&smsg, &odom);


    const auto &p = odom.pose.pose.position;
    ofs << "  - point:\n";
    ofs << "      x: " << p.x << "\n";
    ofs << "      y: " << p.y << "\n";
    ofs << "      z: " << p.z << "\n";

    count++;
  }
  ofs.close();
  RCLCPP_INFO(node->get_logger(), "[OK] 导出 %d 条 waypoint 到 %s", count, out.c_str());

  rclcpp::shutdown();
  return 0;
}
