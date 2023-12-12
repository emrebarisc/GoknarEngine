# Goknar Engine

![alt text](http://www.binarytorgb.com/wp-content/uploads/2023/02/Goknar_Macarena.gif "Goknar Engine Skeletal Animation")

# How to Run

- In order to be able to run the engine, you need to clone submodules as well.
  ```
  git clone https://github.com/emrebarisc/GoknarEngine.git --recurse-submodules
  ```
  
- Goknar folder is for engine source and GameProject folder is for game projects.
- You can see older projects(They may not work due to the changes I made to the engine.) or the current project settings(Always works since I test my works with this settings). You can go through the projects to make your own.
- Then, just generate project files with cmake. (Download cmake, create a Build folder in GameProject folder and, via terminal, call cmake ..)
- It should work in Linux, Windows and Mac Systems. If you cannot run the engine, please let me know.

This engine is capable of:
- Reading FBX objects, XML and image files
- Rendering static, dynamic and skeletal meshes
- Deferred or forward rendering capability
- Shading with 3 different light sources(Directional, point and spot)
- Shadows mapping for all three light sources
- Doing input/output operations
- Timers

Work in progress:
- Implementing a physics engine(Rigid body physics is done, collision detection is WIP)

Missing points:
- Audio Engine(Most probably OpenAL will be integrated)
- And tons of other things(Frustum and occlusion culling, multi-threading etc.)
