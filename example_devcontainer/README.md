# Material Flow Devcontainer

The devcontainer has three layers:

- `Dockerfile.base`: material-flow base tooling.
- `Dockerfile.fleet`: Fleet development tooling, built on the material-flow base layer.
- `Dockerfile.user`: per-user shell setup.

The Fleet layer uses `registry.arculus.it/fleet/fleet/rust-base-or-tools:1.1.0-rust1.95.0` as a source stage for Rust, Cargo tools, certificates, and OR-Tools. It keeps the material-flow base image as the parent layer, then adds Fleet-specific system dependencies and interactive development tooling such as Docker CLI/Compose, Node.js, npm, Nx, Nest CLI, `just`, `k3d`, `kubectl`, `helm`, and `tilt`.

## Image Setup

On container initialization, `.devcontainer/scripts/00_devcontainer_init.sh`:

1. Creates `.devcontainer/.env` from `.devcontainer/.env.example` when needed.
1. Syncs the local platform and Docker runtime settings.
1. Pulls/builds the material-flow base image.
1. Initializes the `fleet` submodule recursively.
1. Pulls/builds the Fleet devcontainer layer.

You can override image handling with:

```bash
DEVCONTAINER_BASE_IMAGE_ACTION=ensure|pull|build|keep
DEVCONTAINER_FLEET_IMAGE_ACTION=ensure|pull|build|keep
```

For manual image builds:

```bash
.devcontainer/scripts/10_build_devcontainer.sh base
.devcontainer/scripts/10_build_devcontainer.sh fleet
.devcontainer/scripts/10_build_devcontainer.sh all
```

Add `--push` to push the selected reusable layer images.

## Fleet Workflows

Full Fleet stack:

```bash
cd /material-flow/fleet
cargo xtask start
```

Build local Fleet compose images:

```bash
cd /material-flow/fleet
docker compose up --build
```

Fleet apps-workspace hybrid development:

```bash
cd /material-flow/fleet/apps-workspace
npm install
npm run start:backend
npm run start:frontend
```

Simulation manager local cluster:

```bash
cd /material-flow/fleet/k8s_simulation_manager
just create-dev-cluster-and-deploy-local-build
```

Analyzer data service local cluster:

```bash
cd /material-flow/fleet/analyzer-data-service
just create-dev-cluster
tilt up
```

## RMF Overlay

The devcontainer installs Open-RMF as a binary underlay (`ros-${ROS_DISTRO}-rmf-dev`)
and imports the Arculus `fleet_adapter`, `rmf_ros2`, and `rmf_internal_msgs`
source overlays into `/material-flow/rmf-ws` from `.devcontainer/rmf_wcs.repos`
during `postCreateCommand`.

Build the Fleet Adapter overlay:

```bash
/material-flow/.devcontainer/scripts/20_build_rmf_overlay.sh
source /material-flow/rmf-ws/install/setup.zsh
```

The helper imports `.devcontainer/rmf_wcs.repos`, installs the small Python
runtime dependencies, and builds the external `fleet_adapter` package against the
binary Jazzy RMF underlay by default. Use `--import-only` to only refresh the
overlay checkout, `--no-rosdep` when dependencies are already installed, or
`--all` when you explicitly want to build all imported RMF source packages.