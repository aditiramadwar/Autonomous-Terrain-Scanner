/**
 * Copyright (c) 2021 Aditi Ramadwar, Arunava Basu
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include "ros/ros.h"
#include "LawnMower.h"

// typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction>
//  MoveBaseClient;

void LawnMower::start(const std_msgs::String::ConstPtr& msg) {
  ROS_DEBUG_STREAM("Successfully received command");
  std::string command = msg->data.c_str();
  ROS_INFO_STREAM("Message heard : " << command);
  move_base_msgs::MoveBaseGoal goal;
  // MoveBaseClient actionClient("move_base", true);
  while (!actionClient.waitForServer(ros::Duration(5.0))) {
    // Wait for move base server
    ROS_INFO("Waiting for the move_base action server to come up");
  }
  NavigationUtils navUtils;
  geometry_msgs::Quaternion qMsg;
  std::vector<std::vector<double>> dummy_pos = {{0.5, 0, 0},
  {0.5, 0, 0}, {0, 0, 90}, {0.5, 0, 0}, {0.5, 0, 0} };
  // Dummy trajectory to test waypoint navigation
  
  for (auto & element : dummy_pos) {
      if (flag=="e_stop") {
        break;
      }
      if (actionClient.isServerConnected()) {
        // convert orietation to quaternion
        current_goal = element;
        qMsg = navUtils.convertToQuaternion(element[2]);
        // set the goal message with desired postion and orientation
        navUtils.setDesiredGoal(goal, element, qMsg);
        ROS_INFO("Sending goal");
        // send the goal message
        navUtils.sendGoal(goal, actionClient);
        // record the status flag
        bool success_flag = navUtils.checkGoalReach(actionClient);
      } else {
          ROS_WARN("Connection to server failed!");
          break;
      }
  }
  if (flag!="e_stop") {
        navUtils.returnToHome(home, actionClient);
      }
  
}

void LawnMower::e_stop(const std_msgs::String::ConstPtr& msg) {
  ROS_DEBUG_STREAM("Successfully received command");
  std::string command = msg->data.c_str();
  ROS_INFO_STREAM("Message heard : " << command);
  NavigationUtils navUtils;
  navUtils.emergencyStop(actionClient);
  
  flag = "e_stop";
}
void LawnMower::pause(const std_msgs::String::ConstPtr& msg) {
  ROS_DEBUG_STREAM("Successfully received command");
  std::string command = msg->data.c_str();
  ROS_INFO_STREAM("Message heard : " << command);
  
}
void LawnMower::resume(const std_msgs::String::ConstPtr& msg) {
  ROS_DEBUG_STREAM("Successfully received command");
  std::string command = msg->data.c_str();
  ROS_INFO_STREAM("Message heard : " << command);
  
}
/**
 * @brief The major function which 
 * starts the lawn mowing routine
 * 
 */
void LawnMower::mow() {
  ros::Subscriber sub_start = node_h.subscribe("alm_start", 1000, &LawnMower::start, this);
  ros::Subscriber sub_e_stop = node_h.subscribe("alm_e_stop", 1000, &LawnMower::e_stop, this);
  ros::Subscriber sub_pause = node_h.subscribe("alm_pause", 1000, &LawnMower::pause, this);
  ros::Subscriber sub_resume = node_h.subscribe("alm_resume", 1000, &LawnMower::resume, this);
  ros::AsyncSpinner spinner(4); // Use 4 threads
  spinner.start();
  ros::waitForShutdown();
}
/**
 * @brief Constructor to set the ros 
 * node hand and set the path to waypoints file
 * 
 * @param n 
 * @param path 
 */
LawnMower::LawnMower(ros::NodeHandle n, std::string path) : actionClient("move_base", true), flag("") {
  node_h = n;
  path_to_waypoints = path;
  home.target_pose.header.frame_id = "map";
  home.target_pose.header.stamp = ros::Time::now();
  home.target_pose.pose.position.x = 0.0145;
  home.target_pose.pose.position.y = 0.023;
  home.target_pose.pose.orientation.x = 0;
  home.target_pose.pose.orientation.y = 0;
  home.target_pose.pose.orientation.z = 0.00632866717679;
  home.target_pose.pose.orientation.w = 0.999979973785;

  ros::spinOnce();
}

/**
 * @brief main function which creates a 
 * new node handle and starts execution
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char **argv) {
  ros::init(argc, argv, "alm");
  ros::NodeHandle ros_node_h;
  std::string path = "../data/waypoints.txt";
  ROS_INFO_STREAM("Starting LawnMower... ");
  LawnMower mower(ros_node_h, path);
  mower.mow();

  return 0;
}
