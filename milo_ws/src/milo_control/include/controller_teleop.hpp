#ifndef MILO_CONTROL__CONTROLLER_TELEOP_HPP_
#define MILO_CONTROL__CONTROLLER_TELEOP_HPP_

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joy.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <std_msgs/msg/u_int8.hpp>
#include "milo_interfaces/msg/auxiliary.hpp"

namespace MiloControl
{
    enum class MotionMode {
        STOP, // Don't send cmd_vel?
        NORMAL,
        PRECISION,
        // TODO (james): Add spot turn and crab drive in the future
    };

    class ControllerTeleop : public rclcpp::Node
    {
    public:    
        ControllerTeleop();
        
    private:

        // Setup
        void getParams();
        void setupPubSubs();

        void cbJoy(const sensor_msgs::msg::Joy::SharedPtr msg);
        double applyDeadzone(double value);
        MotionMode determineMotionMode(const sensor_msgs::msg::Joy::SharedPtr msg);
        void createTwistMsg(const sensor_msgs::msg::Joy::SharedPtr msg, const MiloControl::MotionMode current_mode, geometry_msgs::msg::Twist& cmd_msg);
        void handleAuxiliaryFunctions(const sensor_msgs::msg::Joy::SharedPtr msg);
        void toggleHeadlights();
        void toggleLedStrip();

        // Publisher/subscribers
        rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_sub_;
        rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr motion_mode_pub_;
        rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_msg_pub_;
        rclcpp::Publisher<milo_interfaces::msg::Auxiliary>::SharedPtr auxiliary_pub_;

        // Controller input mapping indices (PS4 controller) from joy package
        // Axes: -1 to 1
        uint8_t l_joystick_l_r_axes_idx_;
        uint8_t l_joystick_u_d_axes_idx_;
        uint8_t r_joystick_l_r_axes_idx_;
        uint8_t r_joystick_u_d_axes_idx_;
        uint8_t l2_throttle_axes_idx_;
        uint8_t r2_throttle_axes_idx_;
        
        // Buttons
        uint8_t x_button_idx_;
        uint8_t o_button_idx_;
        uint8_t square_button_idx_;
        uint8_t triangle_button_idx_;
        uint8_t share_button_idx_;
        uint8_t playstation_button_idx_;
        uint8_t start_button_idx_;
        uint8_t l3_button_idx_;
        uint8_t r3_button_idx_;
        uint8_t l1_button_idx_;
        uint8_t r1_button_idx_;
        uint8_t arrow_up_button_idx_;
        uint8_t arrow_down_button_idx_;
        uint8_t arrow_left_button_idx_;
        uint8_t arrow_right_button_idx_;
        uint8_t touchpad_button_idx_;
        std::vector<int> prev_button_states_;

        // Scaling
        double linear_normal_;
        double angular_normal_;
        double linear_precision_; 
        double angular_precision_;

        // Input
        double deadzone_threshold_;
        double trigger_threshold_;

        // Auxiliary functions
        bool headlights_on_;
        bool led_strip_on_;
    };

} // namespace MiloControl

#endif // MILO_CONTROL__CONTROLLER_TELEOP_HPP_