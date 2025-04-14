#  What is this?
This repo is home of the CRen project as well as projects that uses it. CRen itself is a 3D renderer written in C with Vulkan as the backend API, its goal is to facilitate the development of vulkan applications with an abstraction-layer, offering a default optimizable operation mode, allowing the user to only draw/update/change textures, meshes, etc...

Some projects included on this repo gives a more insight on how to properly setup CRen, a simple HelloWorld with a minimum required setup, and a game engine that gets further used by an Editor.


## Requirements and dependencies
CRen is lightweight, only requiring essencial dependencies, all included and compiled into a static-library with CMake as it's building tool. Projects may use other dependencies and may be written in other programming languages (C++).
* **CRen** (C11 Static-Library): Vulkan 1.1 API, Volk, Stb;
* **HelloWorld** (C11 ConsoleApp): **CRen**, GLFW; 
* **Engine** (C++17 Static-Library): **CRen**, GLFW, ImGui, EnTT;
* **Editor** (C++17 ConsoleApp): **Engine**;

*The user must also compile the shaders located on ```cren/data/shader``` into ```cren/data/shader/compiled``` with the same name and a .spv extension, this is to order to avoid any binary file in this repo. (windows: a bash script exists on the shader's folder to automate this if glslc is on environments variables)*

## License

[MIT](https://choosealicense.com/licenses/mit/)
