del /F/Q/S GoknarEngine.sln
rmdir /Q/S .vs
rmdir /Q/S bin
rmdir /Q/S obj
del /F/Q/S Goknar\Goknar.vcx*
del /F/Q/S Goknar\outsourced\GLAD\GLAD.vcx*
rmdir /Q/S Goknar\outsourced\GLAD\bin
rmdir /Q/S Goknar\outsourced\GLAD\obj
del /F/Q/S Goknar\outsourced\GLFW\GLFW.vcx*
rmdir /Q/S Goknar\outsourced\GLFW\bin
rmdir /Q/S Goknar\outsourced\GLFW\obj
del /F/Q/S Goknar\outsourced\ImGui\ImGui.vcx*
rmdir /Q/S Goknar\outsourced\ImGui\obj

PAUSE