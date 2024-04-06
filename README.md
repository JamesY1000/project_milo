# project_milo
Munich Independent Land Observer

Docker setup:

Development:\
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

Deployment:\
WIP

STL assembly:

![image](https://github.com/JamesY1000/project_milo/assets/107318147/3857e11d-85dc-4792-b498-64467e5a706f)
