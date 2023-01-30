# Goknar Engine

![alt text](http://www.binarytorgb.com/wp-content/uploads/2020/01/Mesh-InstancingTexturingShadingAnd-GenericSceneGeneration.png "Goknar Engine Sample Render")

# How to Run

- In order to be able to run the engine, you need to clone submodules as well.
  ```
  git clone https://github.com/emrebarisc/GoknarEngine.git --recurse-submodules
  ```
  
- Goknar folder is for engine source and GameProject folder is for game projects.
- You can see older projects(They may not work due to the changes I made to the engine.) or the current project settings(Always works since I work with this). You can go through the projects to make your own.
- Then, just generate project files with cmake. (Download cmake, create a Build folder and, via terminal, call cmake ..)
- It should work in Linux, Windows and Mac Systems. If you cannot run the engine, please let me know.

This engine is capable of:
- Reading FBX objects, XML and image files
- Rendering scenes with objects
- Playing 2D animations
- Shading with 3 different light sources(Directional, point and spot)
- Doing input/output operations
- Timers

Work in progress:
- Implementing 3D animations
- Implementing a physics engine
- Implementing shadows

Missing points:
- Audio Engine
