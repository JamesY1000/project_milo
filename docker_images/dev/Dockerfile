FROM osrf/ros:humble-desktop-jammy

# Set working dir inside milo_ws
WORKDIR /root/milo_ws

# Enable QT_X11
ENV QT_X11_NO_MITSHM=1

# install ros2 packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-humble-desktop-full=0.10.0-1* \
    && rm -rf /var/lib/apt/lists/*