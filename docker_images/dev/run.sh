#!/bin/bash

# Start the Docker container with privileged permissions and mount milo_ws directory
docker run --privileged -it -v ~/project_milo/milo_ws:/root/milo_ws milo-docker

# This command is executed after you exit the container.
# Change directory to milo_ws if it exists
if [ -d "~/project_milo/milo_ws" ]; then
    cd ~/project_milo/milo_ws
else
    echo "Directory ~/project_milo/milo_ws does not exist on your local machine."
fi
