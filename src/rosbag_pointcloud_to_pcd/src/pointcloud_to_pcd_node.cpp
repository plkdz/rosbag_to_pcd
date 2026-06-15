#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <rclcpp/serialization.hpp>
#include <rclcpp/serialized_message.hpp>

#include <pcl_conversions/pcl_conversions.h>
#include <pcl/io/pcd_io.h>

#include <rosbag2_cpp/readers/sequential_reader.hpp>
#include <filesystem>

namespace fs = std::filesystem;

class BagToPcdConverter : public rclcpp::Node
{
public:
  std::string topic_name_, bag_path_, pcd_output_path_;

  BagToPcdConverter() : Node("pointcloud_to_pcd_node")
  {
    this->declare_parameter<std::string>("bag_path", "");
    this->declare_parameter<std::string>("topic_name", "/points");
    this->declare_parameter<std::string>("pcd_output_path", "out/pcd/output.pcd");

    this->get_parameter("bag_path", bag_path_);
    this->get_parameter("topic_name", topic_name_);
    this->get_parameter("pcd_output_path", pcd_output_path_);

    RCLCPP_INFO(this->get_logger(), "Bag path: %s", bag_path_.c_str());
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

    while (reader.has_next()) {
      auto bag_message = reader.read_next();

      if (bag_message->topic_name == topic_name_) {
        rclcpp::SerializedMessage serialized_msg(*bag_message->serialized_data);
        sensor_msgs::msg::PointCloud2 pc2_msg;
        serializer.deserialize_message(&serialized_msg, &pc2_msg);

        pcl::PCLPointCloud2 pcl_pc2;
        pcl_conversions::toPCL(pc2_msg, pcl_pc2);

        pcl::PointCloud<pcl::PointXYZ> cloud;
        pcl::fromPCLPointCloud2(pcl_pc2, cloud);

        *global_cloud += cloud;
      }
    }

    if (global_cloud->empty()) {
      RCLCPP_WARN(this->get_logger(), "⚠️ No point cloud messages found on topic: %s", topic_name_.c_str());
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
};

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<BagToPcdConverter>();
  node->convert();
  rclcpp::shutdown();
  return 0;
}
