#ifndef ROS_SERIAL_BRIDGE__ROS_SERIAL_BRIDGE_HPP_
#define ROS_SERIAL_BRIDGE__ROS_SERIAL_BRIDGE_HPP_

#include <mutex>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/u_int8.hpp>
#include <std_msgs/msg/u_int8_multi_array.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include "milo_interfaces/msg/auxiliary.hpp"

namespace RosSerialBridge
{
    constexpr uint16_t CRC16_CCITT_INIT = 0xFFFF;
    constexpr uint16_t CRC16_CCITT_MSB = 0x8000;
    constexpr uint16_t CRC16_CCITT_POLYNOMIAL = 0x1021;

    class RosSerialBridge : public rclcpp::Node
    {
    public:    
        RosSerialBridge();
        
    private:

        // Setup
        void getParams();
        void setupPubSubs();
        void cbMotionMode(const std_msgs::msg::UInt8::SharedPtr msg);
        void cbCmd(const geometry_msgs::msg::Twist::SharedPtr msg);
        void cbAuxiliary(const milo_interfaces::msg::Auxiliary::SharedPtr msg);
        void timerCb();
        uint16_t crc16_ccitt(const uint8_t* data, size_t length);
        bool safetyCheckTimestamp(const int stale_msg_s, 
                                    const rclcpp::Time& latest_motion_mode_time, 
                                    const rclcpp::Time& latest_cmd_time, 
                                    const rclcpp::Time& latest_auxiliary_time);        

        // // Publisher/subscribers
        rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr motion_mode_sub_;
        rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_msg_sub_;
        rclcpp::Subscription<milo_interfaces::msg::Auxiliary>::SharedPtr auxiliary_msg_sub_;
        rclcpp::Publisher<std_msgs::msg::UInt8MultiArray>::SharedPtr serial_pub_;

        rclcpp::TimerBase::SharedPtr control_timer_;
        int control_timer_hz_;
        int stale_msg_s_;
        uint16_t sync_bits_;

        std_msgs::msg::UInt8 latest_motion_mode_;
        geometry_msgs::msg::Twist latest_cmd_;
        milo_interfaces::msg::Auxiliary latest_auxiliary_;
        rclcpp::Time latest_motion_mode_time_;
        rclcpp::Time latest_cmd_time_;
        rclcpp::Time latest_auxiliary_time_;

        std::mutex data_mutex_;


        

    };

} // namespace RosSerialBridge

#endif // ROS_SERIAL_BRIDGE__ROS_SERIAL_BRIDGE_HPP_