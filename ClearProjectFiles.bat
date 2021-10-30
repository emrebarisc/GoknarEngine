del /F/Q/S GoknarEngine.sln
rmdir /Q/S .vs
rmdir /Q/S bin
rmdir /Q/S obj
del /F/Q/S Goknar\Goknar.vcx*
del /F/Q/S GameProject\GameProject.vcx*
del /F/Q/S GameProject\GoknarEngine.dll
del /F/Q/S Goknar\thirdparty\GLAD\GLAD.vcx*
rmdir /Q/S Goknar\thirdparty\GLAD\bin
rmdir /Q/S Goknar\thirdparty\GLAD\obj
del /F/Q/S Goknar\thirdparty\GLFW\GLFW.vcx*
rmdir /Q/S Goknar\thirdparty\GLFW\bin
rmdir /Q/S Goknar\thirdparty\GLFW\obj
del /F/Q/S Goknar\thirdparty\ImGui\ImGui.vcx*
rmdir /Q/S Goknar\thirdparty\ImGui\obj

PAUSE