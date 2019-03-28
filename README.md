# DeltaBetaCL
A fast OpenCL ray marching engine. The ray marching process is accelerated by using multiresolution ray marching, which is a [multigrid method](https://en.wikipedia.org/wiki/Multigrid_method).
It uses OpenCL version 1.2

In the future I'm planning to add:
* Bounding volume hierarchy
* High order nonlinear surface signed distance functions(SDF)
* SDF based general physics engine, for accurate collision detection

## System Dependencies
* OpenCL
* OpenGL
* [AntTweakBar](http://anttweakbar.sourceforge.net/doc/)
* [SFML 2.5.1](https://www.sfml-dev.org)
* [GLM](https://glm.g-truc.net/0.9.9/index.html)

To build the project:
* `git clone https://github.com/MichaelMoroz/DeltaBetaCL`
* Change the library folder paths in "CMakeLists.txt" to the one's you have.
* `cd DeltaBetaCL && mkdir build && cd build && cmake ..`

Note, if you are using Visal Studio, you should set DeltaBetaCLEngine as the startup project.

## Q&A
* Why am I using OpenCL instead of OpenGL compute shaders?
The main reason would be that I have more experience using OpenCL and OpenCL seem's to be more computationally oriented with more advanced memory control. While using it only for ray marching would probably be meaningless, it's definitely better suited to perform physics simulations, since it can even run on the CPU.
And yeah, I'm definitely not using CUDA any time soon.