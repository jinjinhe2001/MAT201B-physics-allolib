# Physics Allolib
## Introduction
Physics Allolib is a toy physical engine written in C++ as a final project for UC Santa Barbara's MAT201B course. It includes some physics simulation like rigid bodies, collisions and cloth simulation, and I implemented octree for  accelerating the collisions. And this project supports UCSB MAT's [AlloSphere](https://allosphere.ucsb.edu/) which is a 3D, 360-degree immersive Research Facility.

## Building
This project is based on [allolib](https://github.com/AlloSphere-Research-Group/allolib/) which supports MacOS/Linux/Windows(some extra [steps](https://github.com/AlloSphere-Research-Group/allolib/) for building on Windows).  
To build Physics Allolib, follow these steps:
```
git clone --recurse-submodules https://github.com/jinjinhe2001/MAT201B-physics-allolib.git
./update.sh
./configure.sh
./run.sh
```
## Result

https://user-images.githubusercontent.com/72654824/229410006-9491a1cb-9ab0-4b46-a83a-ac25c65b9b07.mp4

- alloshpere adapt 
![alloshpere adapt](https://jinjinhe2001.github.io/images/MAT/alloshpere.png)
