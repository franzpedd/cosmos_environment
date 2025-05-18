#  What is this?
This repo is home of the CRen project as well as projects that uses it. CRen itself is a 3D renderer written in C with Vulkan as the backend API, its goal is to facilitate the development of vulkan applications with an abstraction-layer, offering a default optimizable operation mode, allowing the user to only draw/update/change textures, meshes, etc...

Some projects included on this repo gives a more insight on how to properly setup CRen, a simple HelloWorld with a minimum required setup, and a game engine that gets further used by an Editor.

Also, CRen is currently designed to work on whatever platform Vulkan has support. As an example, project_android uses CRen and Engine to create a resistor calculator app.
It's but a simple 2D application, but running using CRen and 3D projects are possible.


## Requirements and dependencies
CRen is lightweight, only requiring essencial dependencies, all included and compiled into a static-library with CMake as it's building tool. Projects may use other dependencies and may be written in other programming languages (C++).
* **CRen** (C11 Shared/Static-Library): Vulkan API, Stb;
* **HelloWorld** (C11 ConsoleApp): **CRen**, GLFW; 
* **Engine** (C++17 Shared-Library): **CRen**, SDL3, ImGui, EnTT;
* **Editor** (C++17 ConsoleApp): **Engine**;
* **Project Android** (C++17 SharedLib): **Engine**;

## License

[MIT](https://choosealicense.com/licenses/mit/)
