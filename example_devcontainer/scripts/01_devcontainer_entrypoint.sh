#!/usr/bin/env bash
set -euo pipefail

WORKSPACE_DIR="/material-flow"

info() {
    echo "[material-flow-entrypoint] $*"
}

warn() {
    echo "[material-flow-entrypoint] $*" >&2
}

DOCKER_GROUP_NAME=""

# Docker socket access
if [[ -S "/var/run/docker.sock" ]]; then
    DOCKER_GROUP_ID=$(stat -c '%g' "/var/run/docker.sock")
    DOCKER_GROUP_NAME="docker-host"
    info "Docker socket group ID: ${DOCKER_GROUP_ID}"

    # Check if a group with this GID already exists
    #shellcheck disable=SC2312
    EXISTING_GROUP=$(getent group | awk -F: -v gid="${DOCKER_GROUP_ID}" '$3 == gid { print $1 }')

    if [[ -n "${EXISTING_GROUP}" ]]; then
        info "Group with GID ${DOCKER_GROUP_ID} already exists: ${EXISTING_GROUP}"
        DOCKER_GROUP_NAME="${EXISTING_GROUP}"
    elif getent group "${DOCKER_GROUP_NAME}" &>/dev/null; then
        DOCKER_GROUP_NAME="docker-host-${DOCKER_GROUP_ID}"
        info "Creating Docker group '${DOCKER_GROUP_NAME}' with GID ${DOCKER_GROUP_ID}..."
        sudo groupadd -g "${DOCKER_GROUP_ID}" "${DOCKER_GROUP_NAME}"
    else
        info "Creating Docker group '${DOCKER_GROUP_NAME}' with GID ${DOCKER_GROUP_ID}..."
        sudo groupadd -g "${DOCKER_GROUP_ID}" "${DOCKER_GROUP_NAME}"
    fi

    # Add the user to the Docker group
    info "Ensuring user '${USER}' can access Docker via '${DOCKER_GROUP_NAME}'..."
    if id "${USER}" &>/dev/null; then
        sudo usermod -aG "${DOCKER_GROUP_NAME}" "${USER}"
    else
        warn "User '${USER}' does not exist. Skipping Docker group setup."
    fi
else
    info "No Docker socket found - skipping Docker group setup."
fi

ensure_writable_dir() {
    local dir="$1"

    sudo mkdir -p "${dir}"
    sudo chown -R "${USER}:${USER}" "${dir}"
}

# Named Docker volumes are mounted as root-owned directories on first use.
ensure_writable_dir /usr/local/cargo/registry
ensure_writable_dir /usr/local/cargo/git
ensure_writable_dir /home/ubuntu/.kube

if [[ -d "${WORKSPACE_DIR}/fleet" ]]; then
    ensure_writable_dir "${WORKSPACE_DIR}/fleet/target"
    ensure_writable_dir "${WORKSPACE_DIR}/fleet/apps-workspace/node_modules"
    ensure_writable_dir "${WORKSPACE_DIR}/fleet/apps-workspace/.nx/cache"
fi

RMF_WORKSPACE_DIR="${RMF_WORKSPACE_DIR:-${WORKSPACE_DIR}/rmf-ws}"
if [[ -d "${RMF_WORKSPACE_DIR}" || -f "${WORKSPACE_DIR}/.devcontainer/rmf_wcs.repos" ]]; then
    mkdir -p "${RMF_WORKSPACE_DIR}/src"
    ensure_writable_dir "${RMF_WORKSPACE_DIR}/build"
    ensure_writable_dir "${RMF_WORKSPACE_DIR}/install"
    ensure_writable_dir "${RMF_WORKSPACE_DIR}/log"
fi

show_fleet_hints() {
    local sentinel="${HOME}/.cache/material-flow-devcontainer/fleet-hints-shown"

    if [[ -f "${sentinel}" || ! -d "${WORKSPACE_DIR}/fleet" ]]; then
        return
    fi

    mkdir -p "$(dirname "${sentinel}")"
    info "Fleet dev tools are available. Common starts: 'cd fleet && cargo xtask start' or 'cd fleet/apps-workspace && npm run start:backend'."
    touch "${sentinel}"
}

show_fleet_hints

show_rmf_hints() {
    local sentinel="${HOME}/.cache/material-flow-devcontainer/rmf-hints-shown"

    if [[ -f "${sentinel}" || ! -f "${WORKSPACE_DIR}/.devcontainer/rmf_wcs.repos" ]]; then
        return
    fi

    mkdir -p "$(dirname "${sentinel}")"
    info "RMF overlay workspace: ${RMF_WORKSPACE_DIR}. Build with '.devcontainer/scripts/20_build_rmf_overlay.sh'. Run Fleet with 'cd fleet && cargo xtask start', then launch the fleet adapter from the built overlay."
    touch "${sentinel}"
}

show_rmf_hints

# Re-enter the command through `sg` so the current shell picks up Docker socket access.
run_command() {
    local command_string

    if [[ $# -eq 0 ]]; then
        set -- zsh
    fi

    if [[ -n "${DOCKER_GROUP_NAME}" ]]; then
        printf -v command_string '%q ' "$@"
        exec sg "${DOCKER_GROUP_NAME}" -c "${command_string}"
    fi

    exec "$@"
}

run_command "$@"