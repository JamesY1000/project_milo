FROM osrf/ros:jazzy-desktop-full

ARG WORKSPACE=milo_ws
WORKDIR /root/$WORKSPACE

ENV NVIDIA_VISIBLE_DEVICES=all
ENV NVIDIA_DRIVER_CAPABILITIES=graphics
ENV QT_X11_NO_MITSHM=1
ENV EDITOR=nano
ENV XDG_RUNTIME_DIR=/tmp

RUN apt-get update && apt-get install -y \
    cmake \
    curl \
    libglu1-mesa-dev \
    nano \
    python3-pip \
    python3-pydantic \
    python3-picamzero \
    ros-jazzy-ros-gz-sim \
    ros-jazzy-ros-gz-bridge \
    ros-jazzy-gz-ros2-control \
    ros-jazzy-joint-state-publisher \
    ros-jazzy-robot-localization \
    ros-jazzy-plotjuggler-ros \
    ros-jazzy-robot-state-publisher \
    ros-jazzy-ros2bag \
    ros-jazzy-rosbag2-storage-default-plugins \
    ros-jazzy-ros2-control \
    ros-jazzy-ros2-controllers \
    ros-jazzy-rqt-tf-tree \
    ros-jazzy-rmw-fastrtps-cpp \
    ros-jazzy-slam-toolbox \
    ros-jazzy-turtlebot3 \
    ros-jazzy-turtlebot3-msgs \
    ros-jazzy-twist-mux \
    ros-jazzy-urdf-tutorial \
    ros-jazzy-usb-cam \
    ros-jazzy-camera-info-manager \
    ros-jazzy-image-transport \
    ros-jazzy-cv-bridge \
    ros-jazzy-rqt-image-view \
    ros-jazzy-xacro \
    ros-jazzy-rviz2 \
    ruby-dev \
    tmux \
    wget \
    xorg-dev \
    zsh

RUN sh -c "$(wget -O- https://github.com/deluan/zsh-in-docker/releases/download/v1.1.2/zsh-in-docker.sh)" -- \
    -p git \
    -p https://github.com/zsh-users/zsh-autosuggestions \
    -p https://github.com/zsh-users/zsh-completions

RUN gem install tmuxinator && \
    wget https://raw.githubusercontent.com/tmuxinator/tmuxinator/master/completion/tmuxinator.zsh -O /usr/local/share/zsh/site-functions/_tmuxinator

RUN apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

RUN echo "source /opt/ros/jazzy/setup.bash" >> /root/.bashrc

RUN echo 'eval "$(register-python-argcomplete3 ros2)"' >> /root/.bashrc
RUN echo 'eval "$(register-python-argcomplete3 colcon)"' >> /root/.bashrc

# add here useful aliases
RUN echo 'alias sw="source ~/milo_ws/install/setup.bash"' >> /root/.bashrc
RUN echo 'alias cb="colcon build"' >> /root/.bashrc
RUN echo 'alias cbps="colcon build --packages-select"' >> /root/.bashrc
RUN echo 'alias cbput="colcon build --packages-up-to"' >> /root/.bashrc
RUN echo 'alias plot="ros2 run plotjuggler plotjuggler"' >> /root/.bashrc