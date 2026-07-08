#include "hardware_interface.hpp"

HardwareInterface::HardwareInterface() : Node("hardware_interface")
{
    getParams();
    setupPubSubs();

    RCLCPP_INFO(this->get_logger(), "Hardware interface node initialised");
}

void HardwareInterface::getParams()
{
    this->declare_parameter("sync_bits", 0xAA55);
    sync_bits_ = static_cast<uint16_t>(this->get_parameter("sync_bits").as_int());

    this->declare_parameter("crc16_ccitt_init", 0xFFFF);
    crc16_ccitt_init_ = static_cast<uint16_t>(this->get_parameter("crc16_ccitt_init").as_int());

    this->declare_parameter("crc16_ccitt_msb", 0x8000);
    crc16_ccitt_msb_ = static_cast<uint16_t>(this->get_parameter("crc16_ccitt_msb").as_int());

    this->declare_parameter("crc16_ccitt_polynomial", 0x1021);
    crc16_ccitt_polynomial_ = static_cast<uint16_t>(this->get_parameter("crc16_ccitt_polynomial").as_int());
}

// Subscribes to rover_command msgs, publishes serialised data to /serial_write (which gets transported via the serial_driver)
void HardwareInterface::setupPubSubs()
{
    rclcpp::QoS cmd_qos(10);
    cmd_qos.best_effort();

    rclcpp::QoS serial_qos(32);
    serial_qos.best_effort();

    rover_command_sub_ = this->create_subscription<milo_interfaces::msg::RoverCommand>(
        "/milo/rover_command", cmd_qos,
        std::bind(&HardwareInterface::cbRoverCommand, this, std::placeholders::_1));

    serial_write_pub_ = this->create_publisher<std_msgs::msg::UInt8MultiArray>(
        "/serial_write", serial_qos);
}

// Takes in rover_command msgs, copies it into a pb msg, serialises pb payload, then publishes
// the framed bites to /serial_write that is then transported via UART by the serial_bridge node
void HardwareInterface::cbRoverCommand(const milo_interfaces::msg::RoverCommand::SharedPtr msg)
{
    RoverCommand rover_command;

    google::protobuf::Timestamp *stamp = rover_command.mutable_stamp();
    stamp->set_seconds(msg->stamp.sec);
    stamp->set_nanos(msg->stamp.nanosec);

    rover_command.set_sequence(msg->sequence);

    rover_command.set_wheel_fl(msg->wheel_fl);
    rover_command.set_wheel_ml(msg->wheel_ml);
    rover_command.set_wheel_rl(msg->wheel_rl);
    rover_command.set_wheel_fr(msg->wheel_fr);
    rover_command.set_wheel_mr(msg->wheel_mr);
    rover_command.set_wheel_rr(msg->wheel_rr);

    rover_command.set_steer_fl(msg->steer_fl);
    rover_command.set_steer_rl(msg->steer_rl);
    rover_command.set_steer_fr(msg->steer_fr);
    rover_command.set_steer_rr(msg->steer_rr);

    rover_command.set_headlights_on(msg->headlights_on);
    rover_command.set_led_strip_on(msg->led_strip_on);
    rover_command.set_enable_motors(msg->enable_motors);
    rover_command.set_estop(msg->estop);

    // Store serialised msg in a buffer
    std::vector<uint8_t> serial_buffer = serialiseMsg(rover_command);
    if (serial_buffer.empty())
    {
        RCLCPP_ERROR(this->get_logger(), "Failed to serialise message: Serial buffer is empty!");
        return;
    }

    // Creates a UInt8MultiArray msg for the serial buffer and publishes it
    std_msgs::msg::UInt8MultiArray serial_msg;
    serial_msg.data = serial_buffer;
    serial_write_pub_->publish(serial_msg);
}

// Takes in a pb rover_command msg, serialises the payload and returns serialised payload as a buffer
std::vector<uint8_t> HardwareInterface::serialiseMsg(RoverCommand rover_command)
{
    std::string payload_bits;
    if (!rover_command.SerializeToString(&payload_bits))
    {
        RCLCPP_ERROR(this->get_logger(), "Failed to serialise RoverCommand data payload!");
        return {};
    }

    std::vector<uint8_t> serial_buffer;

    // [sync][length][payload][crc16]
    // [2bytes]2bytes][nbytes][2bytes]
    // sync bit: 0xAA 0x55
    serial_buffer.push_back(sync_bits_ >> 8);
    serial_buffer.push_back(sync_bits_ & 0xFF);

    size_t payload_length = payload_bits.size();
    serial_buffer.push_back(payload_length >> 8);
    serial_buffer.push_back(payload_length & 0xFF);

    serial_buffer.insert(serial_buffer.end(), payload_bits.begin(), payload_bits.end());

    uint16_t crc = crc16_ccitt(serial_buffer.data(), serial_buffer.size());
    serial_buffer.push_back(crc >> 8);
    serial_buffer.push_back(crc & 0xFF);

    return serial_buffer;
}

// Computes 16-bit checksum over [sync][length][payload] to append at the end as the [crc16]
uint16_t HardwareInterface::crc16_ccitt(const uint8_t *data, size_t length)
{
    uint16_t crc = crc16_ccitt_init_;

    for (size_t i = 0; i < length; i++)
    {
        crc ^= (uint16_t)data[i] << 8;

        for (size_t j = 0; j < 8; j++)
        {
            if (crc & crc16_ccitt_msb_)
            {
                crc = (crc << 1) ^ crc16_ccitt_polynomial_;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<HardwareInterface>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}