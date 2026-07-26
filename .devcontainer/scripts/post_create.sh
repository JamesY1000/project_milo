#!/usr/bin/env bash

# TODO (james): Split this into init.sh, entrypoint.sh, build_devcontainer.sh

set -euo pipefail

ROS_DISTRO="${ROS_DISTRO:-lyrical}"
WORKSPACE="/project_milo/milo_ws"

# ROS setup scripts may read optional vars before defining them.
# Temporarily disable nounset to avoid unbound-variable errors.
set +u
source "/opt/ros/${ROS_DISTRO}/setup.bash"
set -u

# rosdep often needs this after root-owned setup files.
rosdep fix-permissions || true
rosdep update --rosdistro "${ROS_DISTRO}"

if [ -d "${WORKSPACE}/src" ]; then
  cd "${WORKSPACE}"
  sudo -H apt-get update
  rosdep install --from-paths src --ignore-src -r -y --rosdistro "${ROS_DISTRO}"
fi