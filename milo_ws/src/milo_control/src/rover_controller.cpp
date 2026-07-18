#include "rover_controller.hpp"
#include <algorithm>
#include <cmath>


// This node subscribes to cmd_vel (twist), motion_mode (uint8), auxiliary (milo_interfaces/Auxiliary),
// handles rover driving kinematics via computerActuatorTargets
// and creates and publishes a rover_command (milo_interfaces/RoverCommand) message.
// Currently it computes actuator targets as normalised [-1, 1] and [rads], which would eventually need
// to be updated
RoverController::RoverController() : Node("rover_controller")
{
    getParams();
    setupPubSubs();

    got_motion_mode_ = false;
    got_cmd_ = false;

    latest_motion_mode_time_ = this->now();
    latest_cmd_time_ = this->now();
    latest_auxiliary_time_ = this->now();

    enable_motors_ = false;
    estop_ = true;

    int control_timer_period_ms = static_cast<int>(1000.0 / control_timer_hz_);
    control_timer_ = this->create_wall_timer(
        std::chrono::milliseconds(control_timer_period_ms),
        std::bind(&RoverController::timerCb, this));

    RCLCPP_INFO(this->get_logger(), "Rover controller node initialised");
}

void RoverController::getParams()
{
    this->declare_parameter("control_timer_hz", 100);
    control_timer_hz_ = this->get_parameter("control_timer_hz").as_int();

    this->declare_parameter("stale_msg_s", 5);
    stale_msg_s_ = this->get_parameter("stale_msg_s").as_int();

    this->declare_parameter("wheel_base_front", 0.250);
    wheel_base_front_ = this->get_parameter("wheel_base_front").as_double();

    this->declare_parameter("wheel_base_rear", 0.250);
    wheel_base_rear_ = this->get_parameter("wheel_base_rear").as_double();

    this->declare_parameter("track_width", 0.300);
    track_width_ = this->get_parameter("track_width").as_double();

    this->declare_parameter("max_linear_speed", 1.0);
    max_linear_speed_ = this->get_parameter("max_linear_speed").as_double();

    this->declare_parameter("max_steer_angle", 0.785);
    max_steer_angle_ = this->get_parameter("max_steer_angle").as_double();
}

void RoverController::setupPubSubs()
{
    rclcpp::QoS cmd_qos(10);
    cmd_qos.best_effort();

    rclcpp::QoS state_qos(2);
    state_qos.reliable();

    motion_mode_sub_ = this->create_subscription<std_msgs::msg::UInt8>(
        "/milo/motion_mode", state_qos,
        std::bind(&RoverController::cbMotionMode, this, std::placeholders::_1));

    cmd_msg_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
        "/milo/cmd_vel", cmd_qos,
        std::bind(&RoverController::cbCmd, this, std::placeholders::_1));

    auxiliary_msg_sub_ = this->create_subscription<milo_interfaces::msg::Auxiliary>(
        "/milo/auxiliary", state_qos,
        std::bind(&RoverController::cbAuxiliary, this, std::placeholders::_1));

    rover_command_pub_ = this->create_publisher<milo_interfaces::msg::RoverCommand>(
        "/milo/rover_command", cmd_qos);
}

void RoverController::cbMotionMode(const std_msgs::msg::UInt8::SharedPtr msg)
{
    std::lock_guard<std::mutex> lock(data_mutex_);
    latest_motion_mode_ = *msg;
    latest_motion_mode_time_ = this->now();
    got_motion_mode_ = true;
}

void RoverController::cbCmd(const geometry_msgs::msg::Twist::SharedPtr msg)
{
    std::lock_guard<std::mutex> lock(data_mutex_);
    latest_cmd_ = *msg;
    latest_cmd_time_ = this->now();
    got_cmd_ = true;
}

void RoverController::cbAuxiliary(const milo_interfaces::msg::Auxiliary::SharedPtr msg)
{
    std::lock_guard<std::mutex> lock(data_mutex_);
    latest_auxiliary_ = *msg;
    latest_auxiliary_time_ = this->now();}

void RoverController::timerCb()
{
    std_msgs::msg::UInt8 latest_motion_mode;
    geometry_msgs::msg::Twist latest_cmd;
    milo_interfaces::msg::Auxiliary latest_auxiliary;
    rclcpp::Time latest_motion_mode_time;
    rclcpp::Time latest_cmd_time;
    rclcpp::Time latest_auxiliary_time;
    bool got_motion_mode = false;
    bool got_cmd = false;
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        got_motion_mode = got_motion_mode_;
        got_cmd = got_cmd_;
        latest_motion_mode = latest_motion_mode_;
        latest_cmd = latest_cmd_;
        latest_auxiliary = latest_auxiliary_;
        latest_motion_mode_time = latest_motion_mode_time_;
        latest_cmd_time = latest_cmd_time_;
        latest_auxiliary_time = latest_auxiliary_time_;
    }

    bool messages_fresh = safetyCheckTimestamp(stale_msg_s_, got_motion_mode, got_cmd, latest_motion_mode_time, latest_cmd_time, latest_auxiliary_time);

    ActuatorTargets targets;
    if (messages_fresh)
    {
        targets = computeActuatorTargets(latest_cmd, latest_motion_mode);
    }

    milo_interfaces::msg::RoverCommand rover_command;
    rover_command.stamp = this->now();
    rover_command.sequence = control_sequence_;

    rover_command.wheel_fl = static_cast<float>(targets.wheel_fl);
    rover_command.wheel_ml = static_cast<float>(targets.wheel_ml);
    rover_command.wheel_rl = static_cast<float>(targets.wheel_rl);
    rover_command.wheel_fr = static_cast<float>(targets.wheel_fr);
    rover_command.wheel_mr = static_cast<float>(targets.wheel_mr);
    rover_command.wheel_rr = static_cast<float>(targets.wheel_rr);

    rover_command.steer_fl = static_cast<float>(targets.steer_fl);
    rover_command.steer_rl = static_cast<float>(targets.steer_rl);
    rover_command.steer_fr = static_cast<float>(targets.steer_fr);
    rover_command.steer_rr = static_cast<float>(targets.steer_rr);

    rover_command.headlights_on = latest_auxiliary.headlights_on;
    rover_command.led_strip_on = latest_auxiliary.led_strip_on;
    rover_command.enable_motors = enable_motors_;
    rover_command.estop = estop_;

    rover_command_pub_->publish(rover_command);
    control_sequence_++;
}

bool RoverController::safetyCheckTimestamp(const int stale_msg_s,
                                           const bool got_motion_mode,
                                           const bool got_cmd,
                                           const rclcpp::Time& latest_motion_mode_time,
                                           const rclcpp::Time& latest_cmd_time,
                                           const rclcpp::Time& latest_auxiliary_time)
{
    rclcpp::Time now = this->now();
    bool all_messages_fresh = true;
    double throttle_rate_ms = 1000;

    // Fail check if first message has not yet been received
    if (!(got_motion_mode && got_cmd))
    {
        RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), throttle_rate_ms,
            "Skipping: Waiting for first message: motion_mode=%d cmd=%d", 
            got_motion_mode, got_cmd);

        enable_motors_ = false;
        estop_ = true;
        return false;
    }

    // Fail checks for stale msgs
    if ((now - latest_motion_mode_time).seconds() > stale_msg_s)
    {
        RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), throttle_rate_ms,
            "Skipping: Stale motion mode data detected: %.3fs", (now - latest_motion_mode_time).seconds());
        all_messages_fresh = false;
    }

    if ((now - latest_cmd_time).seconds() > stale_msg_s)
    {
        RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), throttle_rate_ms,
            "Skipping: Stale command data detected: %.3fs", (now - latest_cmd_time).seconds());
        all_messages_fresh = false;
    }

    if ((now - latest_auxiliary_time).seconds() > stale_msg_s)
    {
        RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), throttle_rate_ms,
            "Skipping: Stale auxiliary data detected: %.3fs", (now - latest_auxiliary_time).seconds());
    }

    enable_motors_ = all_messages_fresh;
    estop_ = !all_messages_fresh;

    return all_messages_fresh;
}

// TODO (james): Compute physical wheel velocity targets (wheel_fl_velocity_rad_s or m_s)
ActuatorTargets RoverController::computeActuatorTargets(
    const geometry_msgs::msg::Twist& cmd,
    const std_msgs::msg::UInt8& motion_mode)
{
    ActuatorTargets targets;

    if (motion_mode.data == MotionMode::STOP) return targets;

    const double linear_x = cmd.linear.x;
    const double angular_z = cmd.angular.z;
    const double half_track = track_width_ / 2.0;

    // No motion
    if (std::abs(linear_x) < 1e-6 && std::abs(angular_z) < 1e-6) return targets;

    // TODO: Add point-turn mode (linear_x ≈ 0, angular_z ≠ 0)

    // Drive straight
    if (std::abs(angular_z) < 1e-6)
    {
        double speed_normalised = std::clamp(linear_x / max_linear_speed_, -1.0, 1.0);

        targets.wheel_fl = speed_normalised;
        targets.wheel_ml = speed_normalised;
        targets.wheel_rl = speed_normalised;
        targets.wheel_fr = speed_normalised;
        targets.wheel_mr = speed_normalised;
        targets.wheel_rr = speed_normalised;

        return targets;
    }

    // Pure rotation not supported yet
    if (std::abs(linear_x) < 1e-6)
    {
        RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
            "Pure rotation not supported in Ackermann mode — ignoring");
        return targets;
    }

    // ---------------------------------------------------------------
    // 6-wheel Ackermann — leading axle steers
    //
    // Coordinate frame (rover body): x = forward, y = left
    //
    // Forward (linear_x > 0): front wheels steer, rear straight
    // Reverse (linear_x < 0): rear wheels steer, front straight
    //
    // ICR at lateral distance R from centre:
    //   R = linear_x / angular_z
    //   R > 0 → turning left,  R < 0 → turning right
    // ---------------------------------------------------------------

    const bool driving_forward = linear_x > 0.0;
    const double leading_wheelbase = driving_forward ? wheel_base_front_ : wheel_base_rear_;
    const double turn_radius = linear_x / angular_z;

    const double lateral_left = turn_radius - half_track;
    const double lateral_right = turn_radius + half_track;

    // Steer angles (leading axle only)
    double steer_left = std::clamp(std::atan(leading_wheelbase / lateral_left),
                                   -max_steer_angle_, max_steer_angle_);
    double steer_right = std::clamp(std::atan(leading_wheelbase / lateral_right),
                                    -max_steer_angle_, max_steer_angle_);

    // Front two wheels steer
    if (driving_forward)
    {
        targets.steer_fl = steer_left;
        targets.steer_fr = steer_right;
        targets.steer_rl = 0.0;
        targets.steer_rr = 0.0;
    }
    else
    // Back two wheels steer
    {
        targets.steer_fl = 0.0;
        targets.steer_fr = 0.0;
        targets.steer_rl = -steer_left;
        targets.steer_rr = -steer_right;
    }

    // Wheel speeds (proportional to distance from ICR)
    double dist_fl = std::hypot(wheel_base_front_, lateral_left);
    double dist_ml = std::abs(lateral_left);
    double dist_rl = std::hypot(wheel_base_rear_, lateral_left);
    double dist_fr = std::hypot(wheel_base_front_, lateral_right);
    double dist_mr = std::abs(lateral_right);
    double dist_rr = std::hypot(wheel_base_rear_, lateral_right);

    double max_dist = std::max({dist_fl, dist_ml, dist_rl, dist_fr, dist_mr, dist_rr});

    if (max_dist < 1e-6) return targets;

    double base_speed = std::clamp(linear_x / max_linear_speed_, -1.0, 1.0);

    targets.wheel_fl = base_speed * (dist_fl / max_dist);
    targets.wheel_ml = base_speed * (dist_ml / max_dist);
    targets.wheel_rl = base_speed * (dist_rl / max_dist);
    targets.wheel_fr = base_speed * (dist_fr / max_dist);
    targets.wheel_mr = base_speed * (dist_mr / max_dist);
    targets.wheel_rr = base_speed * (dist_rr / max_dist);

    return targets;
}

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<RoverController>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}