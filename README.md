# project_milo
Munich Independent Land Observer

Building the Dockerfiles:

Dev:\
Make sure you are inside the devel directory\
To build the docker image:\
./run.sh -w milo_ws -i milo-devel -b\
or\
mdb

To run the docker image, run the run.sh shell script from inside the same directory:\
./run.sh -w milo_ws -i milo-devel -r\
or\
mdr

Useful aliases inside local setup.bash:\
alias mdb="/home/james/project_milo/docker_images/devel/run.sh -w milo_ws -i milo-devel -b"
Added alias: alias mdr="/home/james/project_milo/docker_images/devel/run.sh -w milo_ws -i milo-devel -r"

Deploy:\
WIP
