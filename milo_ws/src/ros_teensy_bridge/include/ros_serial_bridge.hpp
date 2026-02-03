#ifndef ROS_SERIAL_BRIDGE__ROS_SERIAL_BRIDGE_HPP_
#define ROS_SERIAL_BRIDGE__ROS_SERIAL_BRIDGE_HPP_

#include <mutex>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/u_int8.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include "milo_interfaces/msg/auxiliary.hpp"

namespace RosSerialBridge
{
    class RosSerialBridge : public rclcpp::Node
    {
    public:    
        RosSerialBridge();
        
    private:

        // Setup
        void setupPubSubs();
        void cbMotionMode(const std_msgs::msg::UInt8::SharedPtr msg);
        void cbCmd(const geometry_msgs::msg::Twist::SharedPtr msg);
        void cbAuxiliary(const milo_interfaces::msg::Auxiliary::SharedPtr msg);
        void timerCb();

        // // Publisher/subscribers
        rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr motion_mode_sub_;
        rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_msg_sub_;
        rclcpp::Subscription<milo_interfaces::msg::Auxiliary>::SharedPtr auxiliary_msg_sub_;

        rclcpp::TimerBase::SharedPtr control_timer_;

        geometry_msgs::msg::Twist latest_cmd_;
        std_msgs::msg::UInt8 latest_motion_mode_;
        milo_interfaces::msg::Auxiliary latest_auxiliary_;

        std::mutex data_mutex_;


        

    };

} // namespace RosSerialBridge

#endif // ROS_TEENSY_BRIDGE__ROS_SERIAL_BRIDGE_HPP_