#include "ros_serial_bridge.hpp"
#include "../../milo_proto/generated_code/RoverCommand.pb.h"
#include "../../milo_proto/generated_code/google/protobuf/timestamp.pb.h"

// TODO (james): Clean up above ^^

RosSerialBridge::RosSerialBridge::RosSerialBridge() : Node("ros_serial_bridge_node")
{
    // Setup
    getParams();
    setupPubSubs();

    latest_motion_mode_time_ = this->now();
    latest_cmd_time_ = this->now();
    latest_auxiliary_time_ = this->now();

    // Create wall timer
    int control_timer_period_ms = static_cast<int>(1000.0 / control_timer_hz_);
    control_timer_ = this->create_wall_timer(std::chrono::milliseconds(control_timer_period_ms), std::bind(&RosSerialBridge::timerCb, this));

    RCLCPP_INFO(this->get_logger(), "Ros serial bridge node initialised");
}

void RosSerialBridge::RosSerialBridge::getParams()
{
    this->declare_parameter("control_timer_hz", 100);
    control_timer_hz_ = this->get_parameter("control_timer_hz").as_int();

    this->declare_parameter("stale_msg_s", 5);
    stale_msg_s_ = this->get_parameter("stale_msg_s").as_int();

    this->declare_parameter("sync_bits", 0xAA55);
    sync_bits_ = static_cast<uint16_t>(this->get_parameter("sync_bits").as_int());
}

void RosSerialBridge::RosSerialBridge::setupPubSubs()
{
    // TODO (james): Setup custom milo_qos for different sensors, topics, etc.
    rclcpp::QoS cmd_qos(10);
    cmd_qos.best_effort();

    rclcpp::QoS state_qos(2);
    state_qos.reliable();

    rclcpp::QoS serial_qos(32);
    serial_qos.best_effort();

    motion_mode_sub_ = this->create_subscription<std_msgs::msg::UInt8>(
        "/milo/motion_mode", state_qos, std::bind(&RosSerialBridge::cbMotionMode, this, std::placeholders::_1));

    // TODO (james): Store in milo_common package for topics, frameIDs, names, namespaces?, etc.
    cmd_msg_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
        "/milo/cmd_vel", cmd_qos, std::bind(&RosSerialBridge::cbCmd, this, std::placeholders::_1));

    auxiliary_msg_sub_ = this->create_subscription<milo_interfaces::msg::Auxiliary>(
        "/milo/auxiliary", state_qos, std::bind(&RosSerialBridge::cbAuxiliary, this, std::placeholders::_1));

    serial_pub_ = this->create_publisher<std_msgs::msg::UInt8MultiArray>("/milo/serial_bridge", serial_qos);
}

void RosSerialBridge::RosSerialBridge::cbMotionMode(const std_msgs::msg::UInt8::SharedPtr msg)
{
    std::lock_guard<std::mutex> lock(data_mutex_);
    latest_motion_mode_ = *msg;
    latest_motion_mode_time_ = this->now(); 
}

void RosSerialBridge::RosSerialBridge::cbCmd(const geometry_msgs::msg::Twist::SharedPtr msg)
{
    std::lock_guard<std::mutex> lock(data_mutex_);
    latest_cmd_ = *msg;
    latest_cmd_time_ = this->now();
}

void RosSerialBridge::RosSerialBridge::cbAuxiliary(const milo_interfaces::msg::Auxiliary::SharedPtr msg)
{
    std::lock_guard<std::mutex> lock(data_mutex_);
    latest_auxiliary_ = *msg;
    latest_auxiliary_time_ = this->now();
}

void RosSerialBridge::RosSerialBridge::timerCb()
{
    // Store latest msgs
    std_msgs::msg::UInt8 latest_motion_mode;
    geometry_msgs::msg::Twist latest_cmd;
    milo_interfaces::msg::Auxiliary latest_auxiliary;
    rclcpp::Time latest_motion_mode_time;
    rclcpp::Time latest_cmd_time;
    rclcpp::Time latest_auxiliary_time;

    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        latest_motion_mode = latest_motion_mode_;
        latest_cmd = latest_cmd_;
        latest_auxiliary = latest_auxiliary_;
        latest_motion_mode_time = latest_motion_mode_time_;
        latest_cmd_time = latest_cmd_time_;
        latest_auxiliary_time = latest_auxiliary_time_;
    }

    // Timestamp check - we expect to receive a constant flow of messages
    if (!safetyCheckTimestamp(stale_msg_s_, latest_motion_mode_time, latest_cmd_time, latest_auxiliary_time))
    {
        return;
    };

    // Create RoverCommand proto msg
    RoverCommand rover_command;

    auto now = this->now(); 

    google::protobuf::Timestamp* stamp = rover_command.mutable_stamp();
    stamp->set_seconds(now.seconds());
    stamp->set_nanos(now.nanoseconds() % NANOSECS_PER_SEC);

    rover_command.set_motion_mode(latest_motion_mode.data); 

    rover_command.set_linear_x(latest_cmd.linear.x);
    rover_command.set_linear_y(latest_cmd.linear.y);
    rover_command.set_linear_z(latest_cmd.linear.z);

    rover_command.set_angular_x(latest_cmd.angular.x);
    rover_command.set_angular_y(latest_cmd.angular.y);
    rover_command.set_angular_z(latest_cmd.angular.z);

    rover_command.set_headlights_on(latest_auxiliary.headlights_on);
    rover_command.set_led_strip_on(latest_auxiliary.led_strip_on);

    // Serialise msg: [sync][length][payload][crc16] - bit [2][2][n][2] (big endian)

    // Payload bits
    std::string payload_bits;
    if (!rover_command.SerializeToString(&payload_bits)) // Serialise protobuf payload
    {
        RCLCPP_ERROR(this->get_logger(), "Failed to serialise RoverCommand message!");
        return;
    }

    std::vector<uint8_t> serial_buffer; 

    // Sync bits
    serial_buffer.push_back(sync_bits_ >> 8); // High byte sync - 0xAA
    serial_buffer.push_back(sync_bits_ & 0xFF); // Low byte sync - 0x55

    // Length bits
    size_t payload_length = payload_bits.size();
    serial_buffer.push_back(payload_length >> 8); // High byte length
    serial_buffer.push_back(payload_length & 0xFF); // Low byte length

    // Payload
    serial_buffer.insert(serial_buffer.end(), payload_bits.begin(), payload_bits.end());

    // CRC16 bits
    uint16_t crc = crc16_ccitt(serial_buffer.data(), serial_buffer.size()); // calculate over sync, length, payload
    serial_buffer.push_back(crc >> 8); // High byte crc
    serial_buffer.push_back(crc & 0xFF); // Low byte crc

    // Publish serial msg to serial topic
    std_msgs::msg::UInt8MultiArray serial_msg;
    serial_msg.data = serial_buffer;
    serial_pub_->publish(serial_msg);
}

// CRC16-CCITT implementation (polynomial 0x1021, initial 0xFFFF)
uint16_t RosSerialBridge::RosSerialBridge::crc16_ccitt(const uint8_t* data, size_t length)
{
    // Initial value and polynomial
    uint16_t crc = CRC16_CCITT_INIT; // Initiliased to 0xFFFF

    for (size_t i = 0; i < length; i++) // Loop through each byte in data
    {
        // Convert data byte into 16-bit (2-byte) value and XOR into CRC
        crc ^= (uint16_t)data[i] << 8; // XOR (shift left by 8 bytes) byte into CRC

        for (size_t j = 0; j < 8; j++) // Loop through each bit in the byte
        {
            if (crc & CRC16_CCITT_MSB) // If highest bit (bit 15) is set
            {
                crc = (crc << 1) ^ CRC16_CCITT_POLYNOMIAL; // Then shift left, XOR with polynomial 0x1021
            }
            else
            {
                crc <<= 1; // Otherwise, just shift the bit left
            }
        }
    }

    return crc;
}

bool RosSerialBridge::RosSerialBridge::safetyCheckTimestamp(const int stale_msg_s, 
                                                            const rclcpp::Time& latest_motion_mode_time, 
                                                            const rclcpp::Time& latest_cmd_time, 
                                                            const rclcpp::Time& latest_auxiliary_time)
{
    rclcpp::Time now = this->now();
    bool all_messages_fresh = true;
    double throttle_rate_ms = 1000;

    // Log if message has not been received for a while
    if ((now - latest_motion_mode_time).seconds() > stale_msg_s)
    {
        RCLCPP_WARN_THROTTLE(this->get_logger(),
                            *this->get_clock(),
                            throttle_rate_ms,
                            "Skipping: Stale motion mode data detected: %.3fs", (now - latest_motion_mode_time).seconds()
        );
        all_messages_fresh = false;
    }

    if ((now - latest_cmd_time).seconds() > stale_msg_s)
    {
        RCLCPP_WARN_THROTTLE(this->get_logger(),
                            *this->get_clock(),
                            throttle_rate_ms,
                            "Skipping: Stale command data detected: %.3fs", (now - latest_cmd_time).seconds());
        all_messages_fresh = false;
    }

    if ((now - latest_auxiliary_time).seconds() > stale_msg_s)
    {
        RCLCPP_WARN_THROTTLE(this->get_logger(),
                            *this->get_clock(),
                            throttle_rate_ms,
                            "Skipping: Stale auxiliary data detected: %.3fs", (now - latest_auxiliary_time).seconds());
        all_messages_fresh = false;
    }

    return all_messages_fresh;
}   


int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<RosSerialBridge::RosSerialBridge>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}