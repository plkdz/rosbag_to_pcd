cmake_minimum_required(VERSION 3.8)
project(rosbag_pointcloud_to_pcd)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 查找依赖
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(pcl_ros REQUIRED)
find_package(rosbag2_cpp REQUIRED)

# ==== 构建各个节点 ====

add_executable(pointcloud_to_pcd_node src/pointcloud_to_pcd_node.cpp)
ament_target_dependencies(pointcloud_to_pcd_node
  rclcpp
  sensor_msgs
  pcl_conversions
  pcl_ros
  rosbag2_cpp
)

# ==== 安装 ====

install(TARGETS
  pointcloud_to_pcd_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)
install(FILES README.md
  DESTINATION share/${PROJECT_NAME}
)

# ==== 测试 ====
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_cpplint_FOUND TRUE)
  set(ament_cmake_copyright_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
