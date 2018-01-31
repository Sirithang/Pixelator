Pixelator
=========

Pixelator is a simple program that allow to open FBX file and render them as low resolution sprite
It allow to export animation as spritesheet/multiple image too

(Inspired by Motion Twin worflow for Dead Cell described [here](https://www.gamasutra.com/view/news/313026/Art_Design_Deep_Dive_Using_a_3D_pipeline_for_2D_animation_in_Dead_Cells.php) )

## Building

To build the program, you will need to have :

- glfw
- glew
- assimp

Dependency that are in the project :

- imgui 
- glm


The included Visual Studio project is using my local machine path for include & lib, just change those to your sdks.

For other platform, you'll have to configure your project.

_TODO : someday look at cross-platform tool to define project (e.g. CMake)_