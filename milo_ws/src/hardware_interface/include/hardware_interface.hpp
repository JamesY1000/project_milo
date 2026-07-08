#ifndef HARDWARE_INTERFACE__HARDWARE_INTERFACE_HPP_
#define HARDWARE_INTERFACE__HARDWARE_INTERFACE_HPP_

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/u_int8_multi_array.hpp>
#include "milo_interfaces/msg/rover_command.hpp"

#include "RoverCommand.pb.h"
#include "google/protobuf/timestamp.pb.h"

class HardwareInterface : public rclcpp::Node
{
public:
    HardwareInterface();

private:
    void getParams();
    void setupPubSubs();
    void cbRoverCommand(const milo_interfaces::msg::RoverCommand::SharedPtr msg);
    std::vector<uint8_t> serialiseMsg(RoverCommand rover_command);
    uint16_t crc16_ccitt(const uint8_t* data, size_t length);

    // Publisher/subscribers
    rclcpp::Subscription<milo_interfaces::msg::RoverCommand>::SharedPtr rover_command_sub_;
    rclcpp::Publisher<std_msgs::msg::UInt8MultiArray>::SharedPtr serial_write_pub_;

    uint16_t sync_bits_;
    uint16_t crc16_ccitt_init_;
    uint16_t crc16_ccitt_msb_;
    uint16_t crc16_ccitt_polynomial_;
};

#endif // HARDWARE_INTERFACE__HARDWARE_INTERFACE_HPP_