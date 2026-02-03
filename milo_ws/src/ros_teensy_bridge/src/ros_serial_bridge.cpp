#include "ros_serial_bridge.hpp"
#include "../../milo_proto/generated_code/RoverCommand.pb.h"
#include "../../milo_proto/generated_code/google/protobuf/timestamp.pb.h"

// TODO (james): Clean up above ^^

RosSerialBridge::RosSerialBridge::RosSerialBridge() : Node("ros_serial_bridge_node")
{
    // Setup publishers/subscribers
    setupPubSubs();

    // Create wall timer
    control_timer_ = this->create_wall_timer(std::chrono::milliseconds(10), std::bind(&RosSerialBridge::timerCb, this));



    RCLCPP_INFO(this->get_logger(), "Controller teleop node initialised");
}

void RosSerialBridge::RosSerialBridge::setupPubSubs()
{

    // TODO (james): Setup custom milo_qos for different sensors, topics, etc.
    rclcpp::QoS cmd_qos(10);
    cmd_qos.best_effort();

    rclcpp::QoS state_qos(2);
    state_qos.reliable();

    motion_mode_sub_ = this->create_subscription<std_msgs::msg::UInt8>(
        "/milo/motion_mode", state_qos, std::bind(&RosSerialBridge::cbMotionMode, this, std::placeholders::_1));

    // TODO (james): Store in milo_common package for topics, frameIDs, names, namespaces?, etc.
    cmd_msg_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
        "/milo/cmd_vel", cmd_qos, std::bind(&RosSerialBridge::cbCmd, this, std::placeholders::_1));

    auxiliary_msg_sub_ = this->create_subscription<milo_interfaces::msg::Auxiliary>(
        "/milo/auxiliary", state_qos, std::bind(&RosSerialBridge::cbAuxiliary, this, std::placeholders::_1));
}

void RosSerialBridge::RosSerialBridge::cbMotionMode(const std_msgs::msg::UInt8::SharedPtr msg)
{
    std::lock_guard<std::mutex> lock(data_mutex_);
    latest_motion_mode_ = *msg;
}

void RosSerialBridge::RosSerialBridge::cbCmd(const geometry_msgs::msg::Twist::SharedPtr msg)
{
    std::lock_guard<std::mutex> lock(data_mutex_);
    latest_cmd_ = *msg;
}

void RosSerialBridge::RosSerialBridge::cbAuxiliary(const milo_interfaces::msg::Auxiliary::SharedPtr msg)
{
    std::lock_guard<std::mutex> lock(data_mutex_);
    latest_auxiliary_ = *msg;
}

void RosSerialBridge::RosSerialBridge::timerCb()
{
    // Store latest msgs
    geometry_msgs::msg::Twist latest_cmd;
    std_msgs::msg::UInt8 latest_motion_mode;
    milo_interfaces::msg::Auxiliary latest_auxiliary;

    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        latest_cmd = latest_cmd_;
        latest_motion_mode = latest_motion_mode_;
        latest_auxiliary = latest_auxiliary_;
    }

    // Create RoverCommand proto msg
    RoverCommand rover_command;
    auto now = this->now();
    auto* stamp = new google::protobuf::Timestamp();
    stamp->set_seconds(now.seconds());
    stamp->set_nanos(now.nanoseconds() % 1000000000);
    rover_command.set_allocated_stamp(stamp);

    rover_command.set_motion_mode(latest_motion_mode.data); 

    rover_command.set_linear_x(latest_cmd.linear.x);
    rover_command.set_linear_y(latest_cmd.linear.y);
    rover_command.set_linear_z(latest_cmd.linear.z);

    rover_command.set_angular_x(latest_cmd.angular.x);
    rover_command.set_angular_y(latest_cmd.angular.y);
    rover_command.set_angular_z(latest_cmd.angular.z);

    rover_command.set_headlights_on(latest_auxiliary.headlights_on);
    rover_command.set_led_strip_on(latest_auxiliary.led_strip_on);

    // Serialise msg
    

    // Send over serial

}



int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<RosSerialBridge::RosSerialBridge>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}