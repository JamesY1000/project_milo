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
alias mdb="/home/james/project_milo/docker_images/devel/run.sh -w milo_ws -i milo-devel -b"\
alias mdr="/home/james/project_milo/docker_images/devel/run.sh -w milo_ws -i milo-devel -r"

Useful aliases inside milo_ws docker:\
'alias sw="source ~/milo_ws/install/setup.bash"' >> /root/.bashrc\
'alias cb="colcon build"' >> /root/.bashrc\
'alias cbp="colcon build --packages-select"' >> /root/.bashrc\
'alias plot="ros2 run plotjuggler plotjuggler"' >> /root/.bashrc

Deploy:\
WIP
