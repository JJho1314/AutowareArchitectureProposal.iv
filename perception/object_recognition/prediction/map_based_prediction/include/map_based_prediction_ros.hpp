// Copyright 2018-2019 Autoware Foundation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef MAP_BASED_PREDICTION_ROS_HPP_
#define MAP_BASED_PREDICTION_ROS_HPP_

#include <rclcpp/rclcpp.hpp>

#include <autoware_lanelet2_msgs/msg/map_bin.hpp>
#include <autoware_perception_msgs/msg/dynamic_object.hpp>
#include <autoware_perception_msgs/msg/dynamic_object_array.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <unique_identifier_msgs/msg/uuid.hpp>
#include <visualization_msgs/msg/marker_array.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// lanelet
#include <lanelet2_extension/utility/message_conversion.hpp>
#include <lanelet2_extension/utility/utilities.hpp>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/geometry/BoundingBox.h>
#include <lanelet2_core/geometry/Lanelet.h>
#include <lanelet2_core/geometry/Point.h>
#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>

namespace tf2_ros
{
class Buffer;
class TransformListener;
}  // namespace tf2_ros

namespace lanelet
{
class Lanelet;
class LaneletMap;
using LaneletMapPtr = std::shared_ptr<LaneletMap>;
namespace routing
{
class RoutingGraph;
}  // namespace routing
namespace traffic_rules
{
class TrafficRules;
}  // namespace traffic_rules
}  // namespace lanelet

class MapBasedPrediction;

class MapBasedPredictionROS : public rclcpp::Node
{
private:
  bool has_subscribed_map_;
  double prediction_time_horizon_;
  double prediction_sampling_delta_time_;
  double interpolating_resolution_;
  double debug_accumulated_time_;
  double dist_threshold_for_searching_lanelet_;
  double delta_yaw_threshold_for_searching_lanelet_;
  double sigma_lateral_offset_;
  double sigma_yaw_angle_;

  rclcpp::Subscription<autoware_perception_msgs::msg::DynamicObjectArray>::SharedPtr sub_objects_;
  rclcpp::Subscription<autoware_lanelet2_msgs::msg::MapBin>::SharedPtr sub_map_;
  rclcpp::Publisher<autoware_perception_msgs::msg::DynamicObjectArray>::SharedPtr pub_objects_;
  rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr pub_markers_;

  std::unordered_map<std::string, std::vector<int>> uuid2laneids_;

  std::shared_ptr<tf2_ros::Buffer> tf_buffer_ptr_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_ptr_;

  std::shared_ptr<lanelet::LaneletMap> lanelet_map_ptr_;
  std::shared_ptr<lanelet::routing::RoutingGraph> routing_graph_ptr_;
  std::shared_ptr<lanelet::traffic_rules::TrafficRules> traffic_rules_ptr_;
  std::shared_ptr<MapBasedPrediction> map_based_prediction_;

  bool getSelfPose(geometry_msgs::msg::Pose & self_pose, const std_msgs::msg::Header & header);
  bool getSelfPoseInMap(geometry_msgs::msg::Pose & self_pose);

  double getObjectYaw(const autoware_perception_msgs::msg::DynamicObject & object);
  double calculateLikelihood(
    const std::vector<geometry_msgs::msg::Pose> & path,
    const autoware_perception_msgs::msg::DynamicObject & object);

  void addValidPath(
    const lanelet::routing::LaneletPaths & candidate_paths,
    lanelet::routing::LaneletPaths & valid_paths);

  void objectsCallback(
    const autoware_perception_msgs::msg::DynamicObjectArray::ConstSharedPtr in_objects);
  void mapCallback(const autoware_lanelet2_msgs::msg::MapBin::ConstSharedPtr msg);

  bool getClosestLanelets(
    const autoware_perception_msgs::msg::DynamicObject & object,
    const lanelet::LaneletMapPtr & lanelet_map_ptr,
    std::vector<lanelet::Lanelet> & closest_lanelets, const std::string uuid_string);

  bool checkCloseLaneletCondition(
    const std::pair<double, lanelet::Lanelet> & lanelet,
    const autoware_perception_msgs::msg::DynamicObject & object,
    const lanelet::BasicPoint2d & search_point);

public:
  explicit MapBasedPredictionROS(const rclcpp::NodeOptions & node_options);
};

#endif  // MAP_BASED_PREDICTION_ROS_HPP_
