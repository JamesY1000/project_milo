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

Chassis design:

Dimensions (mm):\
L X W H: 220 X 210 X 105

STL assembly:

![image](https://github.com/JamesY1000/project_milo/assets/107318147/3857e11d-85dc-4792-b498-64467e5a706f)


Bottom layer:

![image](https://github.com/JamesY1000/project_milo/assets/107318147/084eb227-150a-490a-936b-82a819132f0c)

![image](https://github.com/JamesY1000/project_milo/assets/107318147/0ac367b7-0c30-4445-af10-7384eced1500)



Top layer layer:

![image](https://github.com/JamesY1000/project_milo/assets/107318147/bd2f034a-bba6-4405-8c45-f7e22dcfff3d)

![image](https://github.com/JamesY1000/project_milo/assets/107318147/d3f9af0f-56dd-4642-9a6f-5cfe99ee0d41)





