#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Joy
from geometry_msgs.msg import Twist


class PS4TeleopNode(Node):
    def __init__(self):
        super().__init__('ps4_teleop_node')
        
        # Publishers and Subscribers
        self.joy_sub = self.create_subscription(Joy, '/milo/joy', self.joy_callback, 10)
        self.cmd_vel_pub = self.create_publisher(Twist, '/milo/cmd_vel', 10)
        
        # PS4 Controller Button/Axis Indices (typical mapping)
        # You may need to adjust these based on your actual controller mapping
        self.declare_parameter('axis_linear', 1)  # Left stick vertical
        self.declare_parameter('axis_angular', 2)  # Right stick horizontal
        self.declare_parameter('axis_r2', 5)  # R2 trigger (forward enable)
        self.declare_parameter('axis_l2', 4)  # L2 trigger (reverse enable)
        
        # Scaling factors
        self.declare_parameter('scale_linear', 0.5)  # Max linear velocity (m/s)
        self.declare_parameter('scale_angular', 1.0)  # Max angular velocity (rad/s)
        self.declare_parameter('deadzone', 0.1)  # Joystick deadzone
        
        # Get parameters
        self.axis_linear = self.get_parameter('axis_linear').value
        self.axis_angular = self.get_parameter('axis_angular').value
        self.axis_r2 = self.get_parameter('axis_r2').value
        self.axis_l2 = self.get_parameter('axis_l2').value
        self.scale_linear = self.get_parameter('scale_linear').value
        self.scale_angular = self.get_parameter('scale_angular').value
        self.deadzone = self.get_parameter('deadzone').value
        
        self.get_logger().info('PS4 Teleop Node started')
        self.get_logger().info(f'Linear axis: {self.axis_linear}, Angular axis: {self.axis_angular}')
        self.get_logger().info(f'R2 axis: {self.axis_r2}, L2 axis: {self.axis_l2}')
    
    def apply_deadzone(self, value):
        """Apply deadzone to joystick value"""
        if abs(value) < self.deadzone:
            return 0.0
        return value
    
    def joy_callback(self, msg):
        """Process joystick input and publish cmd_vel"""
        twist = Twist()
        
        # PS4 triggers range from 1.0 (not pressed) to -1.0 (fully pressed)
        # Convert to 0.0 (not pressed) to 1.0 (fully pressed)
        r2_value = (1.0 - msg.axes[self.axis_r2]) / 2.0  # Forward enable
        l2_value = (1.0 - msg.axes[self.axis_l2]) / 2.0  # Reverse enable
        
        # Get joystick values
        linear_input = self.apply_deadzone(msg.axes[self.axis_linear])
        angular_input = self.apply_deadzone(msg.axes[self.axis_angular])
        
        # Determine mode based on triggers
        forward_enabled = r2_value > 0.1  # R2 pressed (threshold)
        reverse_enabled = l2_value > 0.1  # L2 pressed (threshold)
        
        if forward_enabled and not reverse_enabled:
            # Forward mode: only allow positive linear velocity
            if linear_input > 0:
                twist.linear.x = linear_input * self.scale_linear * r2_value
            twist.angular.z = angular_input * self.scale_angular
            
        elif reverse_enabled and not forward_enabled:
            # Reverse mode: only allow negative linear velocity
            if linear_input < 0:
                twist.linear.x = linear_input * self.scale_linear * l2_value
            twist.angular.z = angular_input * self.scale_angular
            
        # If both or neither triggers pressed, don't move (safety)
        
        # Publish the command
        self.cmd_vel_pub.publish(twist)


def main(args=None):
    rclpy.init(args=args)
    node = PS4TeleopNode()
    
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()