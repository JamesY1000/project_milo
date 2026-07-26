#include "controller_teleop.hpp"

// This node takes in ps4 controller input and maps each input to an output. It then publishes
// motion_mode_pub_ (UInt8), cmd_msg_pub_ (Twist), auxiliary_pub_ (milo_interfaces/Auxiliary).

ControllerTeleop::ControllerTeleop() : Node("controller_teleop")
{
    // Declare and get parameters
    getParams();

    // Setup publishers/subscribers
    setupPubSubs();

    // Initialise auxiliary functions
    headlights_on_ = false;
    led_strip_on_ = false;
    prev_button_states_.resize(16, 0);

    RCLCPP_INFO(this->get_logger(), "Controller teleop node initialised");
}

void ControllerTeleop::getParams()
{
    // Axes
    this->declare_parameter("controller_mapping.l_joystick_l_r_axes_idx", 0);
    this->declare_parameter("controller_mapping.l_joystick_u_d_axes_idx", 1);
    this->declare_parameter("controller_mapping.r_joystick_l_r_axes_idx", 2);
    this->declare_parameter("controller_mapping.r_joystick_u_d_axes_idx", 3);
    this->declare_parameter("controller_mapping.l2_throttle_axes_idx", 4);
    this->declare_parameter("controller_mapping.r2_throttle_axes_idx", 5);
    l_joystick_l_r_axes_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.l_joystick_l_r_axes_idx").as_int());
    l_joystick_u_d_axes_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.l_joystick_u_d_axes_idx").as_int());
    r_joystick_l_r_axes_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.r_joystick_l_r_axes_idx").as_int());
    r_joystick_u_d_axes_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.r_joystick_u_d_axes_idx").as_int());
    l2_throttle_axes_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.l2_throttle_axes_idx").as_int());
    r2_throttle_axes_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.r2_throttle_axes_idx").as_int());

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
    x_button_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.x_button_idx").as_int());
    o_button_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.o_button_idx").as_int());
    square_button_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.square_button_idx").as_int());
    triangle_button_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.triangle_button_idx").as_int());
    share_button_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.share_button_idx").as_int());
    playstation_button_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.playstation_button_idx").as_int());
    start_button_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.start_button_idx").as_int());
    l3_button_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.l3_button_idx").as_int());
    r3_button_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.r3_button_idx").as_int());
    l1_button_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.l1_button_idx").as_int());
    r1_button_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.r1_button_idx").as_int());
    arrow_up_button_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.arrow_up_button_idx").as_int());
    arrow_down_button_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.arrow_down_button_idx").as_int());
    arrow_left_button_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.arrow_left_button_idx").as_int());
    arrow_right_button_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.arrow_right_button_idx").as_int());
    touchpad_button_idx_ = static_cast<uint8_t>(this->get_parameter("controller_mapping.touchpad_button_idx").as_int());

    // Scaling
    this->declare_parameter("scaling.linear_normal", 1.0);
    this->declare_parameter("scaling.angular_normal", 1.0);
    this->declare_parameter("scaling.linear_precision", 0.5);
    this->declare_parameter("scaling.angular_precision", 0.5);
    linear_normal_ = this->get_parameter("scaling.linear_normal").as_double();
    angular_normal_ = this->get_parameter("scaling.angular_normal").as_double();
    linear_precision_ = this->get_parameter("scaling.linear_precision").as_double();
    angular_precision_ = this->get_parameter("scaling.angular_precision").as_double();

    // Input threshold
    this->declare_parameter("input_threshold.deadzone_threshold", 0.1);
    this->declare_parameter("input_threshold.trigger_threshold", -0.8);
    deadzone_threshold_ = this->get_parameter("input_threshold.deadzone_threshold").as_double();
    trigger_threshold_ = this->get_parameter("input_threshold.trigger_threshold").as_double();
}

void ControllerTeleop::setupPubSubs()
{

    // TODO (james): Setup custom milo_qos for different sensors, topics, etc.
    rclcpp::QoS cmd_qos(10);
    cmd_qos.best_effort();

    rclcpp::QoS state_qos(2);
    state_qos.reliable();

    // TODO (james): Store in milo_common package for topics, frameIDs, names, namespaces?, etc.
    joy_sub_ = this->create_subscription<sensor_msgs::msg::Joy>(
        "/milo/joy", 10, std::bind(&ControllerTeleop::cbJoy, this, std::placeholders::_1));

    motion_mode_pub_ = this->create_publisher<std_msgs::msg::UInt8>("/milo/motion_mode", state_qos);

    cmd_msg_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/milo/cmd_vel", cmd_qos);

    auxiliary_pub_ = this->create_publisher<milo_interfaces::msg::Auxiliary>("/milo/auxiliary", state_qos);
}

void ControllerTeleop::cbJoy(const sensor_msgs::msg::Joy::SharedPtr msg)
{
    if (!msg) return;

    // Check bounds
    const size_t max_axes_idx = std::max({
        static_cast<size_t>(l_joystick_l_r_axes_idx_),
        static_cast<size_t>(l_joystick_u_d_axes_idx_),
        static_cast<size_t>(r_joystick_l_r_axes_idx_),
        static_cast<size_t>(r_joystick_u_d_axes_idx_),
        static_cast<size_t>(l2_throttle_axes_idx_),
        static_cast<size_t>(r2_throttle_axes_idx_)
    });

    const size_t max_button_idx = std::max({
        static_cast<size_t>(x_button_idx_),
        static_cast<size_t>(o_button_idx_),
        static_cast<size_t>(square_button_idx_),
        static_cast<size_t>(triangle_button_idx_),
        static_cast<size_t>(share_button_idx_),
        static_cast<size_t>(playstation_button_idx_),
        static_cast<size_t>(start_button_idx_),
        static_cast<size_t>(l3_button_idx_),
        static_cast<size_t>(r3_button_idx_),
        static_cast<size_t>(l1_button_idx_),
        static_cast<size_t>(r1_button_idx_),
        static_cast<size_t>(arrow_up_button_idx_),
        static_cast<size_t>(arrow_down_button_idx_),
        static_cast<size_t>(arrow_left_button_idx_),
        static_cast<size_t>(arrow_right_button_idx_),
        static_cast<size_t>(touchpad_button_idx_)
    });

    if (msg->axes.size() <= max_axes_idx || msg->buttons.size() <= max_button_idx)
    {
        RCLCPP_WARN(this->get_logger(),
            "Joystick message too small (axes: %zu, buttons: %zu). Expected axes > %zu and buttons > %zu.",
            msg->axes.size(), msg->buttons.size(), max_axes_idx, max_button_idx);

        return;
    }

    MotionMode current_mode;

    // Log joystick values for debugging
    RCLCPP_DEBUG(this->get_logger(),
        "Axes - l_joystick_l_r: %.2f, l_joystick_u_d: %.2f, r_joystick_l_r: %.2f, r_joystick_u_d: %.2f, l2_throttle: %.2f, r2_throttle: %.2f",
        msg->axes[l_joystick_l_r_axes_idx_],
        msg->axes[l_joystick_u_d_axes_idx_],
        msg->axes[r_joystick_l_r_axes_idx_],
        msg->axes[r_joystick_u_d_axes_idx_],
        msg->axes[l2_throttle_axes_idx_],
        msg->axes[r2_throttle_axes_idx_]
    );

    RCLCPP_DEBUG(this->get_logger(),
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
    current_mode = determineMotionMode(msg);

    // Publish motion mode
    std_msgs::msg::UInt8 motion_mode_msg;
    motion_mode_msg.data = static_cast<uint8_t>(current_mode);
    motion_mode_pub_->publish(motion_mode_msg);

    // Create cmd msg
    geometry_msgs::msg::Twist cmd_msg;
    createTwistMsg(msg, current_mode, cmd_msg);

    // Publish cmd_msg
    cmd_msg_pub_->publish(cmd_msg);

    RCLCPP_DEBUG(this->get_logger(), "motion mode: %d", motion_mode_msg.data);

    // Handle auxiliary commands - headlights, led strip
    handleAuxiliaryFunctions(msg);

    RCLCPP_DEBUG(this->get_logger(), "Headlights on: %d, LED strip on: %d", headlights_on_, led_strip_on_);

    // Publish auxiliary ms
    milo_interfaces::msg::Auxiliary auxiliary_msg;
    auxiliary_msg.headlights_on = headlights_on_;
    auxiliary_msg.led_strip_on = led_strip_on_;
    auxiliary_pub_->publish(auxiliary_msg);

}

double ControllerTeleop::applyDeadzone(double value)
{
    if (std::abs(value) < deadzone_threshold_) {
        return 0.0;
    }
    return value;
}


MotionMode ControllerTeleop::determineMotionMode(
    const sensor_msgs::msg::Joy::SharedPtr msg)
{
    // R2 - Normal mode
    // L2 - Precision mode
    // TODO (james): R2 + L2 = crab mode, use only left joystick to control
    // TODO (james): L1 + R1 = spot rotation, use only right joystick to control

    double r2_throttle_value = msg->axes[r2_throttle_axes_idx_];
    double l2_throttle_value = msg->axes[l2_throttle_axes_idx_];
    bool r2_throttle_triggered = false;
    bool l2_throttle_triggered = false;

    // R2 pressed - normal mode
    if (r2_throttle_value <= trigger_threshold_) r2_throttle_triggered = true;

    // L2 pressed - precision mode
    if (l2_throttle_value <= trigger_threshold_) l2_throttle_triggered = true;

    if (r2_throttle_triggered && !l2_throttle_triggered) return MotionMode::NORMAL;
    if (l2_throttle_triggered && !r2_throttle_triggered) return MotionMode::PRECISION;
    if (!r2_throttle_triggered && !l2_throttle_triggered) return MotionMode::STOP;

    // If none of those, return STOP
    return MotionMode::STOP;
}

void ControllerTeleop::createTwistMsg(const sensor_msgs::msg::Joy::SharedPtr msg, const MotionMode current_mode, geometry_msgs::msg::Twist &cmd_msg)
{
    if (current_mode == MotionMode::STOP)
    {
        // Set all values to 0
        cmd_msg.linear.x = 0.0;
        cmd_msg.linear.y = 0.0;
        cmd_msg.linear.z = 0.0;
        cmd_msg.angular.x = 0.0;
        cmd_msg.angular.y = 0.0;
        cmd_msg.angular.z = 0.0;
    }

    else if (current_mode == MotionMode::NORMAL)
    {
        cmd_msg.linear.x = linear_normal_ * applyDeadzone(msg->axes[l_joystick_u_d_axes_idx_]); // Left joystick 1.0/-1.0 u/d
        cmd_msg.linear.y = 0.0;
        cmd_msg.linear.z = 0.0;
        cmd_msg.angular.x = 0.0;
        cmd_msg.angular.y = 0.0;
        cmd_msg.angular.z = angular_normal_ * applyDeadzone(msg->axes[r_joystick_l_r_axes_idx_]); // Right joystick 1.0/-1.0 l/r
    }

    else if (current_mode == MotionMode::PRECISION)
    {
        cmd_msg.linear.x = linear_precision_ * applyDeadzone(msg->axes[l_joystick_u_d_axes_idx_]); // Left joystick 1.0/-1.0 u/d
        cmd_msg.linear.y = 0.0;
        cmd_msg.linear.z = 0.0;
        cmd_msg.angular.x = 0.0;
        cmd_msg.angular.y = 0.0;
        cmd_msg.angular.z = angular_precision_ * applyDeadzone(msg->axes[r_joystick_l_r_axes_idx_]); // Right joystick 1.0/-1.0 l/r
    }
}

void ControllerTeleop::handleAuxiliaryFunctions(const sensor_msgs::msg::Joy::SharedPtr msg)
{

    // Make size of prev_button_states the same as current button size
    if (prev_button_states_.size() != msg->buttons.size())
    {
        prev_button_states_.assign(msg->buttons.size(), 0);
    }

    // Only toggle if button is currently pressed once (skips the toggle if button is held)
    if (msg->buttons[arrow_up_button_idx_] && !prev_button_states_[arrow_up_button_idx_])
    {
        toggleHeadlights();
    }

    if (msg->buttons[arrow_right_button_idx_] && !prev_button_states_[arrow_right_button_idx_])
    {
        toggleLedStrip();
    }

    // Update prev button states
    prev_button_states_ = msg->buttons;

    return;
}

void ControllerTeleop::toggleHeadlights()
{
    headlights_on_ = !headlights_on_;
}

void ControllerTeleop::toggleLedStrip()
{
    led_strip_on_ = !led_strip_on_;
}

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ControllerTeleop>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}