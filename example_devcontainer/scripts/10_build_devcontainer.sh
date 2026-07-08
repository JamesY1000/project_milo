#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
DEVCONTAINER_DIR="$(cd "${SCRIPT_DIR}/.." >/dev/null 2>&1 && pwd)"
COMPOSE_FILE="${DEVCONTAINER_DIR}/docker-compose.yml"
ENV_FILE="${DEVCONTAINER_DIR}/.env"

DEFAULT_REGISTRY="registry.arculus.it/material-flow/material-flow"
DEFAULT_IMAGE_TAG="latest"
DEFAULT_PLATFORM=""

COMMAND=""
PUSH_IMAGE=false
PULL_CACHE=false
PLATFORM_OVERRIDE=""
REGISTRY_OVERRIDE=""
TAG_OVERRIDE=""

info() {
    printf '[material-flow-devcontainer] %s\n' "$*"
}

warn() {
    printf '[material-flow-devcontainer] %s\n' "$*" >&2
}

die() {
    printf '[material-flow-devcontainer] ERROR: %s\n' "$*" >&2
    exit 1
}

detect_platform() {
    local arch
    arch="$(uname -m)"

    case "${arch}" in
        x86_64) printf 'amd64\n' ;;
        aarch64|arm64) printf 'arm64\n' ;;
        *) die "Unsupported architecture: ${arch}" ;;
    esac
}

normalize_platform() {
    case "$1" in
        amd64|linux/amd64) printf 'amd64\n' ;;
        arm64|linux/arm64) printf 'arm64\n' ;;
        *) die "Unsupported platform: $1" ;;
    esac
}

usage() {
    cat <<'EOF'
Usage:
    .devcontainer/scripts/10_build_devcontainer.sh <command> [options]

Commands:
    base              Build the reusable base image target locally.
    fleet             Build the reusable Fleet image target locally.
    dev               Build the devcontainer user image target locally.
    all               Build the base image, Fleet image, then the dev image.
    ensure-base       Ensure the resolved base image exists locally: keep it if present,
                                        otherwise pull it, and build it only if the pull fails.
    ensure-fleet      Ensure the resolved Fleet image exists locally: keep it if present,
                                        otherwise pull it, and build it only if the pull fails.
    prepare-base      Resolve a base image action from DEVCONTAINER_BASE_IMAGE_ACTION
                                        or an interactive prompt, then apply it.
    prepare-fleet     Resolve a Fleet image action from DEVCONTAINER_FLEET_IMAGE_ACTION
                                        or an interactive prompt, then apply it.
    print-base-image  Print the resolved base image reference (registry/name:tag).
    print-fleet-image Print the resolved Fleet image reference (registry/name:tag).
    pull-base         Pull the resolved base image reference from the registry.
    pull-fleet        Pull the resolved Fleet image reference from the registry.
    interactive       Run prepare-base and prepare-fleet, then build the dev image.

Options:
  --push                  Push images after building.
  --pull-cache            Try to pull latest cache images before building.
    --platform <amd64|arm64>  Set the image platform suffix and DOCKER_DEFAULT_PLATFORM for the build.
  --tag <tag>             Override IMAGE_TAG for this invocation.
  --registry <registry>   Override MATERIAL_FLOW_REGISTRY for this invocation.
  -h, --help              Show this help.

The script never mutates .devcontainer/.env. Use .devcontainer/.env
or exported environment variables for local overrides.
Reusable image names include the resolved platform, for example
material-flow-devcontainer-base-arm64:latest.
EOF
}

load_env_file() {
    local file="$1"

    if [[ -f "$file" ]]; then
        # shellcheck disable=SC1090
        set -a && source "$file" && set +a
    fi
}

compose() {
    docker compose -f "${COMPOSE_FILE}" "$@"
}

base_image_name() {
    printf '%s/material-flow-devcontainer-base-%s:%s\n' \
        "${MATERIAL_FLOW_REGISTRY:-$DEFAULT_REGISTRY}" \
        "${PLATFORM:?PLATFORM is required}" \
        "${IMAGE_TAG:-$DEFAULT_IMAGE_TAG}"
}

user_image_name() {
    printf '%s/material-flow-devcontainer-user-%s:%s\n' \
        "${MATERIAL_FLOW_REGISTRY:-$DEFAULT_REGISTRY}" \
        "${PLATFORM:?PLATFORM is required}" \
        "${IMAGE_TAG:-$DEFAULT_IMAGE_TAG}"
}

fleet_image_name() {
    printf '%s/material-flow-devcontainer-fleet-%s:%s\n' \
        "${MATERIAL_FLOW_REGISTRY:-$DEFAULT_REGISTRY}" \
        "${PLATFORM:?PLATFORM is required}" \
        "${IMAGE_TAG:-$DEFAULT_IMAGE_TAG}"
}

fleet_image_name() {
    printf '%s/material-flow-devcontainer-fleet-%s:%s\n' \
        "${MATERIAL_FLOW_REGISTRY:-$DEFAULT_REGISTRY}" \
        "${PLATFORM:?PLATFORM is required}" \
        "${IMAGE_TAG:-$DEFAULT_IMAGE_TAG}"
}

pull_cache() {
    info "Warming registry cache if available."
    docker pull "$(base_image_name)" || warn "Could not pull base image cache; continuing with local build."
    docker pull "$(fleet_image_name)" || warn "Could not pull Fleet image cache; continuing with local build."
    docker pull "$(user_image_name)" || warn "Could not pull user image cache; continuing with local build."
}

build_base() {
    info "Building devcontainer base image."
    compose build material-flow-devcontainer-base

    if [[ "${PUSH_IMAGE}" == true ]]; then
        info "Pushing devcontainer base image."
        compose push material-flow-devcontainer-base
    fi
}

build_fleet() {
    info "Building devcontainer Fleet image."
    compose build material-flow-devcontainer-fleet

    if [[ "${PUSH_IMAGE}" == true ]]; then
        info "Pushing devcontainer Fleet image."
        compose push material-flow-devcontainer-fleet
    fi
}

build_dev() {
    info "Building devcontainer user image."
    compose build material-flow-devcontainer-user

    if [[ "${PUSH_IMAGE}" == true ]]; then
        info "Pushing devcontainer user image."
        compose push material-flow-devcontainer-user
    fi
}

pull_base() {
    info "Pulling base image: $(base_image_name)"
    docker pull "$(base_image_name)"
}

pull_fleet() {
    info "Pulling Fleet image: $(fleet_image_name)"
    docker pull "$(fleet_image_name)"
}

ensure_base() {
    if docker image inspect "$(base_image_name)" >/dev/null 2>&1; then
        info "Base image already exists locally: $(base_image_name)"
        return 0
    fi

    warn "Base image is not available locally: $(base_image_name)"
    if pull_base; then
        return 0
    fi

    warn "Could not pull base image; building it locally."
    build_base
}

ensure_fleet() {
    if docker image inspect "$(fleet_image_name)" >/dev/null 2>&1; then
        info "Fleet image already exists locally: $(fleet_image_name)"
        return 0
    fi

    warn "Fleet image is not available locally: $(fleet_image_name)"
    if pull_fleet; then
        return 0
    fi

    warn "Could not pull Fleet image; building it locally."
    build_fleet
}

select_base_action() {
    local base_image_name configured_action choice
    local input=/dev/stdin output=/dev/stdout
    local attempts=0 max_attempts=3

    configured_action="${DEVCONTAINER_BASE_IMAGE_ACTION:-}"
    if [[ -n "${configured_action}" ]]; then
        configured_action="${configured_action,,}"
        case "${configured_action}" in
            build|keep|pull|ensure)
                info "Using DEVCONTAINER_BASE_IMAGE_ACTION=${configured_action}." >&2
                printf '%s\n' "${configured_action}"
                return 0
                ;;
            *) warn "Ignoring unsupported DEVCONTAINER_BASE_IMAGE_ACTION='${configured_action}'." ;;
        esac
    fi

    base_image_name="$(base_image_name)"

    if ! { : < /dev/tty; } 2>/dev/null || ! { : > /dev/tty; } 2>/dev/null; then
        warn "No interactive terminal. Defaulting to pull (set DEVCONTAINER_BASE_IMAGE_ACTION=build, pull, keep, or ensure to override)."
        printf 'pull\n'
        return 0
    fi

    input=/dev/tty
    output=/dev/tty

    while (( attempts < max_attempts )); do
        printf '\n[material-flow-devcontainer] Base image: %s\n' "${base_image_name}" > "${output}"
        printf '[material-flow-devcontainer] Choose how to prepare the base image:\n' > "${output}"
        printf '  [P] Pull latest image from the registry\n' > "${output}"
        printf '  [B] Build image locally\n' > "${output}"
        printf '  [K] Keep current image as-is\n' > "${output}"
        printf '[material-flow-devcontainer] Selection [P/B/K]: ' > "${output}"

        if ! read -r choice < "${input}"; then
            break
        fi

        case "${choice,,}" in
            p|pull|"") printf 'pull\n'; return 0 ;;
            b|build) printf 'build\n'; return 0 ;;
            k|keep) printf 'keep\n'; return 0 ;;
        esac

        printf "[material-flow-devcontainer] Please enter 'P', 'B', or 'K'.\n" > "${output}"
        attempts=$((attempts + 1))
    done

    warn "No valid selection read. Defaulting to pull."
    printf 'pull\n'
}

prepare_base_image() {
    local action="$1"

    case "${action}" in
        ensure) ensure_base ;;
        build) build_base ;;
        pull) pull_base ;;
        keep) info "Keeping current base image as-is." ;;
        *) die "Unsupported base image action: ${action}" ;;
    esac
}

select_fleet_action() {
    local fleet_image_name configured_action choice
    local input=/dev/stdin output=/dev/stdout
    local attempts=0 max_attempts=3

    configured_action="${DEVCONTAINER_FLEET_IMAGE_ACTION:-}"
    if [[ -n "${configured_action}" ]]; then
        configured_action="${configured_action,,}"
        case "${configured_action}" in
            build|keep|pull|ensure)
                info "Using DEVCONTAINER_FLEET_IMAGE_ACTION=${configured_action}." >&2
                printf '%s\n' "${configured_action}"
                return 0
                ;;
            *) warn "Ignoring unsupported DEVCONTAINER_FLEET_IMAGE_ACTION='${configured_action}'." ;;
        esac
    fi

    fleet_image_name="$(fleet_image_name)"

    if ! { : < /dev/tty; } 2>/dev/null || ! { : > /dev/tty; } 2>/dev/null; then
        warn "No interactive terminal. Defaulting to build (set DEVCONTAINER_FLEET_IMAGE_ACTION=build, pull, keep, or ensure to override)."
        printf 'build\n'
        return 0
    fi

    input=/dev/tty
    output=/dev/tty

    while (( attempts < max_attempts )); do
        printf '\n[material-flow-devcontainer] Fleet image: %s\n' "${fleet_image_name}" > "${output}"
        printf '[material-flow-devcontainer] Choose how to prepare the Fleet image:\n' > "${output}"
        printf '  [P] Pull latest image from the registry\n' > "${output}"
        printf '  [B] Build image locally\n' > "${output}"
        printf '  [K] Keep current image as-is\n' > "${output}"
        printf '[material-flow-devcontainer] Selection [P/B/K, default B]: ' > "${output}"

        if ! read -r choice < "${input}"; then
            break
        fi

        case "${choice,,}" in
            p|pull) printf 'pull\n'; return 0 ;;
            b|build|"") printf 'build\n'; return 0 ;;
            k|keep) printf 'keep\n'; return 0 ;;
        esac

        printf "[material-flow-devcontainer] Please enter 'P', 'B', or 'K'.\n" > "${output}"
        attempts=$((attempts + 1))
    done

    warn "No valid selection read. Defaulting to build."
    printf 'build\n'
}

prepare_fleet_image() {
    local action="$1"

    case "${action}" in
        ensure) ensure_fleet ;;
        build) build_fleet ;;
        pull) pull_fleet ;;
        keep) info "Keeping current Fleet image as-is." ;;
        *) die "Unsupported Fleet image action: ${action}" ;;
    esac
}

interactive() {
    prepare_base_image "$(select_base_action)"
    prepare_fleet_image "$(select_fleet_action)"

    build_dev
}

if [[ $# -eq 0 ]]; then
    usage
    exit 1
fi

COMMAND="$1"
shift

while [[ $# -gt 0 ]]; do
    case "$1" in
        --push)
            PUSH_IMAGE=true
            ;;
        --pull-cache)
            PULL_CACHE=true
            ;;
        --platform)
            shift
            [[ $# -gt 0 ]] || die "--platform requires a value"
            PLATFORM_OVERRIDE="$1"
            ;;
        --tag)
            shift
            [[ $# -gt 0 ]] || die "--tag requires a value"
            TAG_OVERRIDE="$1"
            ;;
        --registry)
            shift
            [[ $# -gt 0 ]] || die "--registry requires a value"
            REGISTRY_OVERRIDE="$1"
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            die "Unknown option: $1"
            ;;
    esac
    shift
done

load_env_file "${ENV_FILE}"

export MATERIAL_FLOW_REGISTRY="${MATERIAL_FLOW_REGISTRY:-$DEFAULT_REGISTRY}"
export IMAGE_TAG="${IMAGE_TAG:-$DEFAULT_IMAGE_TAG}"
export CACHE_IMAGE_TAG="${CACHE_IMAGE_TAG:-$DEFAULT_IMAGE_TAG}"
export NODE_VERSION="${NODE_VERSION:-20}"
export FLEET_RUST_BASE_IMAGE="${FLEET_RUST_BASE_IMAGE:-registry.arculus.it/fleet/fleet/rust-base-or-tools:1.1.0-rust1.95.0}"
export FLEET_NODE_VERSION="${FLEET_NODE_VERSION:-24.9.0}"
export FLEET_NPM_VERSION="${FLEET_NPM_VERSION:-11.6.0}"
PLATFORM="${PLATFORM:-${DEFAULT_PLATFORM}}"

if [[ -n "${REGISTRY_OVERRIDE}" ]]; then
    export MATERIAL_FLOW_REGISTRY="${REGISTRY_OVERRIDE}"
fi

if [[ -n "${TAG_OVERRIDE}" ]]; then
    export IMAGE_TAG="${TAG_OVERRIDE}"
fi

if [[ -n "${PLATFORM_OVERRIDE}" ]]; then
    PLATFORM="${PLATFORM_OVERRIDE}"
fi

if [[ -z "${PLATFORM}" ]]; then
    PLATFORM="$(detect_platform)"
fi

PLATFORM="$(normalize_platform "${PLATFORM}")"
export PLATFORM

export DOCKER_BUILDKIT=1
export COMPOSE_DOCKER_CLI_BUILD=1
export DOCKER_DEFAULT_PLATFORM="linux/${PLATFORM}"
info "Using PLATFORM=${PLATFORM}."
info "Using DOCKER_DEFAULT_PLATFORM=${DOCKER_DEFAULT_PLATFORM}."

if [[ "${PULL_CACHE}" == true ]]; then
    pull_cache
fi

case "${COMMAND}" in
    base) build_base ;;
    fleet) build_fleet ;;
    dev) build_dev ;;
    all) build_base && build_fleet && build_dev ;;
    ensure-base) ensure_base ;;
    ensure-fleet) ensure_fleet ;;
    prepare-base) prepare_base_image "$(select_base_action)" ;;
    prepare-fleet) prepare_fleet_image "$(select_fleet_action)" ;;
    print-base-image) base_image_name ;;
    print-fleet-image) fleet_image_name ;;
    pull-base) pull_base ;;
    pull-fleet) pull_fleet ;;
    interactive) interactive ;;
    -h|--help) usage ;;
    *) die "Unknown command: ${COMMAND}" ;;
esac