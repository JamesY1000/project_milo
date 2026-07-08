#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEVCONTAINER_DIR="$(dirname "$SCRIPT_DIR")"
REPO_ROOT="$(dirname "$DEVCONTAINER_DIR")"
BUILD_SCRIPT="${SCRIPT_DIR}/10_build_devcontainer.sh"
ENV_TEMPLATE="${DEVCONTAINER_DIR}/.env.example"
ENV_FILE="${DEVCONTAINER_DIR}/.env"

info() { printf '[material-flow-init] %s\n' "$*"; }
warn() { printf '[material-flow-init] %s\n' "$*" >&2; }
error() { printf '[material-flow-init] %s\n' "$*" >&2; }

prepare_host_cache_dirs() {
  mkdir -p "${HOME}/.cache/pre-commit"
}

read_env_value() {
  local file="$1" key="$2"
  awk -F= -v key="$key" '$1 == key { print substr($0, length(key) + 2); exit }' "$file"
}

set_env_value() {
  local file="$1" key="$2" value="$3" tmp_file
  tmp_file="$(mktemp)"
  awk -v key="$key" -v value="$value" '
    BEGIN { updated = 0 }
    $0 ~ "^" key "=" {
      print key "=" value
      updated = 1
      next
    }
    { print }
    END { if (!updated) print key "=" value }
  ' "$file" > "$tmp_file"
  mv "$tmp_file" "$file"
}

# Detect Platform
detect_platform() {
  local arch
  arch="$(uname -m)"

  case "$arch" in
    x86_64) printf 'amd64\n' ;;
    aarch64|arm64) printf 'arm64\n' ;;
    *)
      error "Unsupported architecture: ${arch}"
      exit 1
      ;;
  esac
}

sync_platform() {
  local platform
  platform="$(detect_platform)"
  set_env_value "$ENV_FILE" "PLATFORM" "$platform"
  info "Using PLATFORM=${platform}."
}

sync_container_runtime() {
  if ! command -v docker >/dev/null 2>&1; then
    warn "Docker not found; leaving CONTAINER_RUNTIME unchanged."
    return
  fi

  local docker_runtimes
  if ! docker_runtimes="$(docker info --format '{{json .Runtimes}}' 2>/dev/null)"; then
    warn "Could not inspect Docker runtimes; leaving CONTAINER_RUNTIME unchanged."
    return
  fi

  if grep -q '"nvidia"' <<<"$docker_runtimes"; then
    info "NVIDIA runtime available."
    return
  fi

  if [[ "$(read_env_value "$ENV_FILE" "CONTAINER_RUNTIME")" == "nvidia" ]]; then
    set_env_value "$ENV_FILE" "CONTAINER_RUNTIME" "runc"
    warn "NVIDIA runtime unavailable; set CONTAINER_RUNTIME=runc."
  fi
}

prepare_env_file() {
  if [[ -f "${ENV_FILE}" ]]; then
    info "Using existing ${ENV_FILE}."
    return
  fi

  if [[ ! -f "${ENV_TEMPLATE}" ]]; then
    error "Missing ${ENV_TEMPLATE}."
    exit 1
  fi

  cp "${ENV_TEMPLATE}" "${ENV_FILE}"
  info "Created ${ENV_FILE}."
}

sync_fleet_submodule() {
  if [[ ! -f "${REPO_ROOT}/.gitmodules" ]]; then
    warn "No .gitmodules file found. Skipping fleet submodule setup."
    return
  fi

  if ! git -C "$REPO_ROOT" config --file .gitmodules --get submodule.fleet.path >/dev/null; then
    warn "No fleet submodule is configured. Skipping fleet submodule setup."
    return
  fi

  info "Ensuring fleet submodule and nested submodules are initialized."
  if ! git -C "$REPO_ROOT" submodule update --init --recursive fleet; then
    error "Could not initialize the fleet submodule. Check GitLab SSH access and retry."
    exit 1
  fi
}

# Main
main() {
  prepare_host_cache_dirs
  prepare_env_file
  sync_platform
  sync_container_runtime
  bash "${BUILD_SCRIPT}" prepare-base
  sync_fleet_submodule
  bash "${BUILD_SCRIPT}" prepare-fleet
  info "Initialization complete."
}

main "$@"