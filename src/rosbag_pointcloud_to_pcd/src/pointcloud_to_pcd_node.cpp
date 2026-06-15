#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <rclcpp/serialization.hpp>
#include <rclcpp/serialized_message.hpp>

#include <pcl_conversions/pcl_conversions.h>
#include <pcl/io/pcd_io.h>

#include <rosbag2_cpp/readers/sequential_reader.hpp>
#include <algorithm>
#include <cctype>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

class BagToPcdConverter : public rclcpp::Node
{
public:
  std::string topic_name_;
  std::string topic_names_csv_;
  std::string bag_path_;
  std::string pcd_output_path_;
  std::vector<std::string> topic_names_;
  std::unordered_set<std::string> topic_name_set_;

  BagToPcdConverter() : Node("pointcloud_to_pcd_node")
  {
    this->declare_parameter<std::string>("bag_path", "");
    this->declare_parameter<std::string>("topic_name", "/points");
    this->declare_parameter<std::string>("topic_names_csv", "");
    this->declare_parameter<std::string>("pcd_output_path", "out/pcd/output.pcd");

    this->get_parameter("bag_path", bag_path_);
    this->get_parameter("topic_name", topic_name_);
    this->get_parameter("topic_names_csv", topic_names_csv_);
    this->get_parameter("pcd_output_path", pcd_output_path_);

    topic_names_ = resolve_target_topics();
    topic_name_set_.insert(topic_names_.begin(), topic_names_.end());

    RCLCPP_INFO(this->get_logger(), "Bag path: %s", bag_path_.c_str());
    RCLCPP_INFO(this->get_logger(), "Target topics: %s", join_topics(topic_names_).c_str());
  }

  void convert()
  {
    if (bag_path_.empty()) {
      RCLCPP_ERROR(this->get_logger(), "必须提供参数: bag_path");
      return;
    }

    rosbag2_cpp::readers::SequentialReader reader;
    reader.open({bag_path_, "sqlite3"}, {});
    rclcpp::Serialization<sensor_msgs::msg::PointCloud2> serializer;

    pcl::PointCloud<pcl::PointXYZ>::Ptr global_cloud(new pcl::PointCloud<pcl::PointXYZ>);
    const std::size_t total_message_count = read_total_message_count_from_metadata();
    std::size_t scanned_message_count = 0U;
    std::size_t matched_message_count = 0U;

    if (total_message_count > 0U) {
      print_progress(scanned_message_count, total_message_count, matched_message_count);
    }

    while (reader.has_next()) {
      auto bag_message = reader.read_next();
      ++scanned_message_count;

      if (topic_name_set_.count(bag_message->topic_name) > 0U) {
        rclcpp::SerializedMessage serialized_msg(*bag_message->serialized_data);
        sensor_msgs::msg::PointCloud2 pc2_msg;
        serializer.deserialize_message(&serialized_msg, &pc2_msg);

        pcl::PCLPointCloud2 pcl_pc2;
        pcl_conversions::toPCL(pc2_msg, pcl_pc2);

        pcl::PointCloud<pcl::PointXYZ> cloud;
        pcl::fromPCLPointCloud2(pcl_pc2, cloud);

        *global_cloud += cloud;
        ++matched_message_count;
      }

      if (total_message_count > 0U &&
        (scanned_message_count == total_message_count ||
        scanned_message_count % 10U == 0U))
      {
        print_progress(scanned_message_count, total_message_count, matched_message_count);
      }
    }

    if (total_message_count > 0U) {
      std::cout << std::endl;
    }

    if (global_cloud->empty()) {
      RCLCPP_WARN(
        this->get_logger(),
        "未在目标话题中找到点云消息: %s",
        join_topics(topic_names_).c_str());
      return;
    }

    const fs::path output_path(pcd_output_path_);
    if (output_path.has_parent_path()) {
      fs::create_directories(output_path.parent_path());
    }

    if (pcl::io::savePCDFileBinary(pcd_output_path_, *global_cloud) != 0) {
      RCLCPP_ERROR(this->get_logger(), "PCD 保存失败: %s", pcd_output_path_.c_str());
      return;
    }

    RCLCPP_INFO(this->get_logger(), "✅ Saved concatenated cloud to: %s", pcd_output_path_.c_str());
  }

private:
  std::size_t read_total_message_count_from_metadata() const
  {
    const fs::path metadata_path = fs::path(bag_path_) / "metadata.yaml";
    std::ifstream metadata_stream(metadata_path);
    if (!metadata_stream.is_open()) {
      RCLCPP_WARN(
        this->get_logger(),
        "无法读取 metadata.yaml，跳过进度条: %s",
        metadata_path.string().c_str());
      return 0U;
    }

    std::string line;
    while (std::getline(metadata_stream, line)) {
      const std::string trimmed_line = trim(line);
      const std::string prefix = "message_count:";
      if (trimmed_line.rfind(prefix, 0U) == 0U) {
        const std::string value = trim(trimmed_line.substr(prefix.size()));
        try {
          return static_cast<std::size_t>(std::stoull(value));
        } catch (const std::exception &) {
          RCLCPP_WARN(
            this->get_logger(),
            "metadata.yaml 中的 message_count 解析失败，跳过进度条");
          return 0U;
        }
      }
    }

    RCLCPP_WARN(this->get_logger(), "metadata.yaml 中未找到 message_count，跳过进度条");
    return 0U;
  }

  static void print_progress(
    const std::size_t scanned_message_count,
    const std::size_t total_message_count,
    const std::size_t matched_message_count)
  {
    constexpr std::size_t bar_width = 40U;
    const double ratio = total_message_count == 0U ? 0.0 :
      static_cast<double>(scanned_message_count) / static_cast<double>(total_message_count);
    const std::size_t filled_width = std::min(
      bar_width,
      static_cast<std::size_t>(ratio * static_cast<double>(bar_width)));

    std::ostringstream bar_stream;
    bar_stream << '\r' << '[';
    for (std::size_t index = 0U; index < bar_width; ++index) {
      bar_stream << (index < filled_width ? '#' : '-');
    }
    bar_stream << "] "
      << std::fixed << std::setprecision(1) << (ratio * 100.0) << "% "
      << "(" << scanned_message_count << "/" << total_message_count << ") "
      << "matched=" << matched_message_count;

    std::cout << bar_stream.str() << std::flush;
  }

  static std::string trim(const std::string & input)
  {
    std::string value = input;
    value.erase(
      value.begin(),
      std::find_if(
        value.begin(), value.end(),
        [](unsigned char ch) { return !std::isspace(ch); }));
    value.erase(
      std::find_if(
        value.rbegin(), value.rend(),
        [](unsigned char ch) { return !std::isspace(ch); }).base(),
      value.end());
    return value;
  }

  std::vector<std::string> resolve_target_topics() const
  {
    std::vector<std::string> topics;

    std::istringstream stream(topic_names_csv_);
    std::string item;
    while (std::getline(stream, item, ',')) {
      item = trim(item);
      if (!item.empty()) {
        topics.push_back(item);
      }
    }

    if (topics.empty()) {
      const std::string fallback_topic = trim(topic_name_);
      if (!fallback_topic.empty()) {
        topics.push_back(fallback_topic);
      }
    }

    return topics;
  }

  static std::string join_topics(const std::vector<std::string> & topics)
  {
    std::ostringstream stream;
    for (std::size_t index = 0U; index < topics.size(); ++index) {
      if (index > 0U) {
        stream << ", ";
      }
      stream << topics[index];
    }
    return stream.str();
  }
};

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<BagToPcdConverter>();
  node->convert();
  rclcpp::shutdown();
  return 0;
}
