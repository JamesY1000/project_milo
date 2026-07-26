#!/usr/bin/env bash
set -euo pipefail

WORKSPACE_DIR="${WORKSPACE_DIR:-/material-flow}"
RMF_WORKSPACE_DIR="${RMF_WORKSPACE_DIR:-${WORKSPACE_DIR}/rmf-ws}"
RMF_REPOS_FILE="${RMF_REPOS_FILE:-${WORKSPACE_DIR}/.devcontainer/rmf_wcs.repos}"
ROS_DISTRO="${ROS_DISTRO:-jazzy}"

RUN_ROSDEP=1
IMPORT_ONLY=0
BUILD_ALL=0
COLCON_ARGS=()
PIP_PACKAGES=("nudged" "paho-mqtt>=2.1.0")
ROSDEP_SKIP_KEYS=("pybind11-json-dev" "rmf_reservation_msgs")
APT_FALLBACK_PACKAGES=("ros-${ROS_DISTRO}-pybind11-json-vendor")

info() {
    printf '[material-flow-rmf-build] %s\n' "$*"
}

warn() {
    printf '[material-flow-rmf-build] %s\n' "$*" >&2
}

usage() {
    cat <<'EOF'
Usage: 20_build_rmf_overlay.sh [options] [-- <colcon args>]

Options:
  --import-only   Import/update repositories and stop before dependency install/build.
  --all           Build all imported RMF overlay packages.
  --no-rosdep     Skip rosdep install.
  -h, --help      Show this help.

Any arguments after -- are forwarded to `colcon build`.
EOF
}

source_ros_underlay() {
    if [[ ! -f "/opt/ros/${ROS_DISTRO}/setup.bash" ]]; then
        warn "Missing ROS setup: /opt/ros/${ROS_DISTRO}/setup.bash"
        exit 1
    fi

    # ROS setup scripts may reference unset variables.
    set +u
    # shellcheck source=/dev/null
    source "/opt/ros/${ROS_DISTRO}/setup.bash"
    set -u
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --import-only)
            IMPORT_ONLY=1
            shift
            ;;
        --all)
            BUILD_ALL=1
            shift
            ;;
        --no-rosdep)
            RUN_ROSDEP=0
            shift
            ;;
        --)
            shift
            COLCON_ARGS=("$@")
            break
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            COLCON_ARGS+=("$1")
            shift
            ;;
    esac
done

if [[ ! -f "${RMF_REPOS_FILE}" ]]; then
    warn "Missing RMF repos file: ${RMF_REPOS_FILE}"
    exit 1
fi

source_ros_underlay

mkdir -p "${RMF_WORKSPACE_DIR}/src"
cp "${RMF_REPOS_FILE}" "${RMF_WORKSPACE_DIR}/rmf_wcs.repos"

if ! command -v vcs >/dev/null 2>&1; then
    warn "The 'vcs' command is unavailable. Install python3-vcstool first."
    exit 1
fi

info "Importing RMF repositories into ${RMF_WORKSPACE_DIR}."
vcs import --skip-existing "${RMF_WORKSPACE_DIR}" < "${RMF_WORKSPACE_DIR}/rmf_wcs.repos"

FLEET_ADAPTER_PATH="${RMF_WORKSPACE_DIR}/src/fleet_adapter_template/fleet_adapter"
if [[ ! -d "${FLEET_ADAPTER_PATH}" ]]; then
    warn "RMF overlay import is incomplete. Missing: ${FLEET_ADAPTER_PATH}"
    warn "Check network access and repository permissions for ${RMF_REPOS_FILE}."
    exit 1
fi

if [[ "${BUILD_ALL}" == "1" ]]; then
    for required_repo in \
        "${RMF_WORKSPACE_DIR}/src/rmf_ros2" \
        "${RMF_WORKSPACE_DIR}/src/rmf_internal_msgs"; do
        if [[ ! -d "${required_repo}" ]]; then
            warn "RMF overlay import is incomplete. Missing: ${required_repo}"
            warn "Check network access and repository permissions for ${RMF_REPOS_FILE}."
            exit 1
        fi
    done
fi

if [[ "${IMPORT_ONLY}" == "1" ]]; then
    info "Import-only mode complete."
    exit 0
fi

if [[ "${RUN_ROSDEP}" == "1" ]]; then
    if ! command -v rosdep >/dev/null 2>&1; then
        warn "The 'rosdep' command is unavailable."
        exit 1
    fi
    if [[ ! -d "${HOME}/.ros/rosdep/sources.cache" ]]; then
        info "Initializing rosdep cache with rosdep update."
        rosdep update
    fi
    info "Installing Fleet Adapter Python dependencies."
    python3 -m pip install --user --break-system-packages "${PIP_PACKAGES[@]}"
    if [[ "${BUILD_ALL}" == "1" ]]; then
        info "Installing full RMF overlay apt fallback dependencies."
        sudo -H apt-get update
        sudo -H apt-get install -y "${APT_FALLBACK_PACKAGES[@]}"
        info "Installing full RMF overlay ROS dependencies with rosdep."
        rosdep install \
            --from-paths "${RMF_WORKSPACE_DIR}/src" \
            --ignore-src \
            --rosdistro "${ROS_DISTRO}" \
            --skip-keys "${ROSDEP_SKIP_KEYS[*]}" \
            -yr
    else
        info "Installing Fleet Adapter ROS dependencies with rosdep."
        rosdep install --from-paths "${FLEET_ADAPTER_PATH}" --ignore-src --rosdistro "${ROS_DISTRO}" -yr
    fi
fi

if ! command -v colcon >/dev/null 2>&1; then
    warn "The 'colcon' command is unavailable."
    exit 1
fi

info "Building RMF overlay with colcon."
cd "${RMF_WORKSPACE_DIR}"
if [[ "${BUILD_ALL}" == "1" ]]; then
    colcon build --symlink-install "${COLCON_ARGS[@]}"
else
    colcon build --symlink-install --packages-select fleet_adapter "${COLCON_ARGS[@]}"
fi

info "RMF overlay build complete. Source ${RMF_WORKSPACE_DIR}/install/setup.bash before running RMF commands."