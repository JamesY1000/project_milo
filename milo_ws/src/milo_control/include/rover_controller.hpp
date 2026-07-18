#ifndef ROVER_CONTROLLER__ROVER_CONTROLLER_HPP_
#define ROVER_CONTROLLER__ROVER_CONTROLLER_HPP_

#include <mutex>
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <std_msgs/msg/u_int8.hpp>
#include "milo_interfaces/msg/auxiliary.hpp"
#include "milo_interfaces/msg/rover_command.hpp"

enum MotionMode : uint8_t
{
    STOP = 0,
    NORMAL = 1,
    PRECISION = 2
};

struct ActuatorTargets
{
    // Drive wheels [-1.0, 1.0]
    double wheel_fl = 0.0;
    double wheel_ml = 0.0;
    double wheel_rl = 0.0;
    double wheel_fr = 0.0;
    double wheel_mr = 0.0;
    double wheel_rr = 0.0;

    // Steering angles [rad]
    double steer_fl = 0.0;
    double steer_rl = 0.0;
    double steer_fr = 0.0;
    double steer_rr = 0.0;
};

class RoverController : public rclcpp::Node
{
public:    
    RoverController();
    
private:
    // Setup
    void getParams();
    void setupPubSubs();
    void cbMotionMode(const std_msgs::msg::UInt8::SharedPtr msg);
    void cbCmd(const geometry_msgs::msg::Twist::SharedPtr msg);
    void cbAuxiliary(const milo_interfaces::msg::Auxiliary::SharedPtr msg);
    void timerCb();
    bool safetyCheckTimestamp(const int stale_msg_s,
                                            const bool got_motion_mode,
                                            const bool got_cmd,
                                            const bool got_auxiliary,
                                            const rclcpp::Time& latest_motion_mode_time,
                                            const rclcpp::Time& latest_cmd_time,
                                            const rclcpp::Time& latest_auxiliary_time);
    ActuatorTargets computeActuatorTargets(const geometry_msgs::msg::Twist& cmd, 
                                            const std_msgs::msg::UInt8& motion_mode);

    // Publishers/subscribers
    rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr motion_mode_sub_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_msg_sub_;
    rclcpp::Subscription<milo_interfaces::msg::Auxiliary>::SharedPtr auxiliary_msg_sub_;
    rclcpp::Publisher<milo_interfaces::msg::RoverCommand>::SharedPtr rover_command_pub_;

    // Timer
    rclcpp::TimerBase::SharedPtr control_timer_;
    int control_timer_hz_;
    int stale_msg_s_;

    // Data
    std::mutex data_mutex_;
    std_msgs::msg::UInt8 latest_motion_mode_;
    geometry_msgs::msg::Twist latest_cmd_;
    milo_interfaces::msg::Auxiliary latest_auxiliary_;
    rclcpp::Time latest_motion_mode_time_;
    rclcpp::Time latest_cmd_time_;
    rclcpp::Time latest_auxiliary_time_;
    int control_sequence_{0};
    bool enable_motors_;
    bool estop_;

    // Rover geometry
    double wheel_base_front_;
    double wheel_base_rear_;
    double track_width_;
    double max_linear_speed_;
    double max_steer_angle_;

    // Startup/initialisation flags to avoid false freshness
    bool got_motion_mode_;
    bool got_cmd_;
    bool got_auxiliary_;
};

#endif // ROVER_CONTROLLER__ROVER_CONTROLLER_HPP_