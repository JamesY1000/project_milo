#!/usr/bin/env python3

from geometry_msgs.msg import TransformStamped
from nav_msgs.msg import Odometry
from tf2_ros import TransformBroadcaster

import rclpy
from rclpy.node import Node
from rclpy.qos import qos_profile_sensor_data

# TODO: (james) Refactor Alberto's node (write your own)

class OdomToTF(Node):
    """ Subscribes to a odometry message and publishes a corresponding TF.
    Takes incoming Odometry messages to convert them to TF messages. 
    The topic of the TF is automatically taken from the Odometry message header and child_frame_id
    (typically base_footprint or base_link).
    * Subscribes: `/odom` : Odometry
    * Publishes `/tf` : TransformStamped
    """

    def __init__(self):
        super().__init__('odom_to_tf')
        self.joint_sub = self.create_subscription(msg_type=Odometry, topic='odometry/filtered', callback=self._odom_callback, qos_profile=qos_profile_sensor_data)
        self._bc = TransformBroadcaster(self)
        # Cache
        self._tf = TransformStamped()

    def _update_frames(self, msg: Odometry) -> bool:
        # If this is the first message, take the frame ids for the map TF
        if not self._tf.header.frame_id:
            self._tf.header.frame_id = msg.header.frame_id
            self.get_logger().info(f"Odometry->TF parent frame is \"{self._tf.header.frame_id}\"")
        # Select child frame ID
        if not self._tf.child_frame_id:
            if msg.child_frame_id:
                self._tf.child_frame_id = msg.child_frame_id
            else:
                self.get_logger().info("No child frame in odometry message - skipping.")
                return False
            self.get_logger().info(f"Odometry->TF child frame is \"{self._tf.child_frame_id}\"")
        return True

    def _odom_callback(self, msg: Odometry):
        """ Take the odom message and convert it to a odom->base_link transform """
        if not self._update_frames(msg):
            return
        # Copy metadata
        self._tf.header.stamp = msg.header.stamp
        # Copy TF data
        self._tf.transform.translation.x = msg.pose.pose.position.x
        self._tf.transform.translation.y = msg.pose.pose.position.y
        self._tf.transform.translation.z = msg.pose.pose.position.z
        self._tf.transform.rotation = msg.pose.pose.orientation

        self._bc.sendTransform([self._tf])


def main(args=None):
    rclpy.init(args=args)
    odom_to_tf = OdomToTF()
    rclpy.spin(odom_to_tf)
    odom_to_tf.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
