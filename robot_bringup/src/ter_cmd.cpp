#include "ros/ros.h"
#include <cstdlib> // for system()
using namespace std;
    
    int main(int argc, char** argv)
    {
      // Call roslaunch command to run your launch file
      ros::init(argc, argv, "ter_cmd");
      std::string autonomous_cmd = "robot_bringup learning_me.launch"; // Replace with the path to your actual launch file
      std::string command = "roslaunch " + autonomous_cmd;
      int result = system(command.c_str());
      if (result == -1)
      {
        ROS_ERROR("Failed to execute roslaunch command");
        return 1;
      }
    
      return 0;
    }