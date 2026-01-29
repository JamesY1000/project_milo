#include "controller_teleop.hpp"

MiloControl::ControllerTeleop::ControllerTeleop() : Node("controller_teleop_node")
{
    // Declare and get parameters
    get_params();
    
    // Setup publishers/subscribers
    setup_pub_subs();

    RCLCPP_INFO(this->get_logger(), "Controller teleop node initialised");
}

void MiloControl::ControllerTeleop::get_params()
{
    // Axes
    this->declare_parameter("controller_mapping.l_joystick_l_r_axes_idx", 0);
    this->declare_parameter("controller_mapping.l_joystick_u_d_axes_idx", 1);
    this->declare_parameter("controller_mapping.r_joystick_l_r_axes_idx", 2);
    this->declare_parameter("controller_mapping.r_joystick_u_d_axes_idx", 3);
    this->declare_parameter("controller_mapping.l2_throttle_axes_idx", 4);
    this->declare_parameter("controller_mapping.r2_throttle_axes_idx", 5);

    // TODO (james): Finish these param declarations off
    l_joystick_l_r_axes_idx_ = this->get_parameter("controller_mapping.l_joystick_l_r_axes_idx").as_int();
    l_joystick_u_d_axes_idx_ = this->get_parameter("controller_mapping.l_joystick_u_d_axes_idx").as_int();
    r_joystick_l_r_axes_idx_;
    r_joystick_u_d_axes_idx_;
    l2_throttle_axes_idx_;
    r2_throttle_axes_idx_;

    // Buttons
    this->declare_parameter("controller_mapping.x_button_idx", 0);
    this->declare_parameter("controller_mapping.o_button_idx", 1);
    this->declare_parameter("controller_mapping.square_button_idx", 2);
    this->declare_parameter("controller_mapping.triangle_button_idx", 3);
    this->declare_parameter("controller_mapping.share_button_idx", 4);
    this->declare_parameter("controller_mapping.playstation_button_idx", 5);
    this->declare_parameter("controller_mapping.start_button_idx", 6);
    this->declare_parameter("controller_mapping.l3_button_idx", 7);
    this->declare_parameter("controller_mapping.r3_button_idx", 8);
    this->declare_parameter("controller_mapping.l1_button_idx", 9);
    this->declare_parameter("controller_mapping.r1_button_idx", 10);
    this->declare_parameter("controller_mapping.arrow_up_button_idx", 11);
    this->declare_parameter("controller_mapping.arrow_down_button_idx", 12);
    this->declare_parameter("controller_mapping.arrow_left_button_idx", 13);
    this->declare_parameter("controller_mapping.arrow_right_button_idx", 14);
    this->declare_parameter("controller_mapping.touchpad_button_idx", 15);

    // Scaling
    this->declare_parameter("scaling.linear_normal", 1.0);
    this->declare_parameter("scaling.angular_normal", 1.0);
    this->declare_parameter("scaling.linear_precision", 0.3);
    this->declare_parameter("scaling.angular_precision", 0.3);

    // Input threshold
    this->declare_parameter("input_threshold.deadzone_threshold", 0.1);
    this->declare_parameter("input_threshold.trigger_threshold", -0.8);
}

void MiloControl::ControllerTeleop::setup_pub_subs()
{
    // TODO (james): Store in milo_common package for topics, frameIDs, names, namespaces?, etc.
    joy_sub_ = this->create_subscription<sensor_msgs::msg::Joy>(
        "/milo/joy", 10, std::bind(&ControllerTeleop::cb_joy, this, std::placeholders::_1));

    cmd_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/milo/cmd_vel", 1);
    motion_mode_pub_ = this->create_publisher<std_msgs::msg::UInt8>("/milo/motion_mode", 1);

}

void MiloControl::ControllerTeleop::cb_joy(const sensor_msgs::msg::Joy::SharedPtr msg)
{
    if (!msg) return;

    // Log joystick values for debugging
    RCLCPP_INFO(this->get_logger(), 
        "Axes - l_joystick_l_r: %.2f, l_joystick_u_d: %.2f, r_joystick_l_r: %.2f, r_joystick_u_d: %.2f, l2_throttle: %.2f, r2_throttle: %.2f", 
        msg->axes[l_joystick_l_r_axes_idx_],
        msg->axes[l_joystick_u_d_axes_idx_],
        msg->axes[r_joystick_l_r_axes_idx_],
        msg->axes[r_joystick_u_d_axes_idx_],
        msg->axes[l2_throttle_axes_idx_],
        msg->axes[r2_throttle_axes_idx_]
    );

    RCLCPP_INFO(this->get_logger(), 
        "Buttons - x: %d, o: %d, square: %d, triangle: %d, share: %d, playstation_button: %d, start: %d, "
        "l3: %d, r3: %d, l1: %d, r1: %d, arrow_up: %d, arrow_down: %d, arrow_left: %d, arrow_right: %d, touchpad_pressed: %d",
        msg->buttons[x_button_idx_],
        msg->buttons[o_button_idx_],
        msg->buttons[square_button_idx_],
        msg->buttons[triangle_button_idx_],
        msg->buttons[share_button_idx_],
        msg->buttons[playstation_button_idx_],
        msg->buttons[start_button_idx_],
        msg->buttons[l3_button_idx_],
        msg->buttons[r3_button_idx_],
        msg->buttons[l1_button_idx_],
        msg->buttons[r1_button_idx_],
        msg->buttons[arrow_up_button_idx_],
        msg->buttons[arrow_down_button_idx_],
        msg->buttons[arrow_left_button_idx_],
        msg->buttons[arrow_right_button_idx_],
        msg->buttons[touchpad_button_idx_]
    );

    // Determine motion mode
    MotionMode mode = determine_motion_mode(msg);

    // Create/send cmd_velocities based on motion mode
    // Also send motion mode at same rate
    // Also send aux control? at same rate - make a diagram maybe of communication protocol
    // That's it - no more complicated than that?
    // THen, ros_teensy_bridge reads cmd_vel, along with motion mode and serialises that abd sends to teensy
    // Do I really need a motion mode publisher? Can teensy infer enough through serialisation of cmd_Vel alone? Probs not right? It still needs aux_control
    
}

double MiloControl::ControllerTeleop::apply_deadzone(double value)
{
    if (std::abs(value) < deadzone_threshold_) {
        return 0.0;
    }
    return value;
}


MiloControl::MotionMode MiloControl::ControllerTeleop::determine_motion_mode(
    const sensor_msgs::msg::Joy::SharedPtr msg)
{
    // R2 - Normal mode
    // L2 - Precision mode
    
}

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<MiloControl::ControllerTeleop>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}