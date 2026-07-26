FROM osrf/ros:jazzy-desktop-noble



ARG WORKSPACE=milo_ws
WORKDIR /root/$WORKSPACE

ENV NVIDIA_VISIBLE_DEVICES=all
ENV NVIDIA_DRIVER_CAPABILITIES=graphics
ENV QT_X11_NO_MITSHM=1
ENV EDITOR=nano
ENV XDG_RUNTIME_DIR=/tmp
# ROS doesn't recognize the docker shells as terminals so force colored output
ENV RCUTILS_COLORIZED_OUTPUT=1

# install ros2 packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-jazzy-desktop-full=0.11.0-1* \
    && rm -rf /var/lib/apt/lists/*

# Install other packages
RUN apt-get update && apt-get install -y \
    cmake \
    curl \
    libglu1-mesa-dev \
    nano \
    python3-pip \
    python3-pydantic \
    python3-picamzero \
    ruby-dev \
    tmux \
    wget \
    xorg-dev \
    zsh

RUN apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

RUN echo "source /opt/ros/jazzy/setup.bash" >> /root/.bashrc

# add here useful aliases
RUN echo 'alias sw="source ~/milo_ws/install/setup.bash"' >> /root/.bashrc
RUN echo 'alias cb="colcon build"' >> /root/.bashrc
RUN echo 'alias cbps="colcon build --packages-select"' >> /root/.bashrc
RUN echo 'alias cbput="colcon build --packages-up-to"' >> /root/.bashrc
RUN echo 'alias plot="ros2 run plotjuggler plotjuggler"' >> /root/.bashrc