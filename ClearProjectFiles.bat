del /F/Q/S GoknarEngine.sln
rmdir /Q/S .vs
rmdir /Q/S bin
rmdir /Q/S obj
del /F/Q/S Goknar\Goknar.vcx%
del /F/Q/S outsourced\GLAD\GLAD.vcx%
rmdir /Q/S outsourced\GLAD\bin
rmdir /Q/S outsourced\GLAD\obj
del /F/Q/S outsourced\GLFW\GLFW.vcx%
rmdir /Q/S outsourced\GLFW\bin
rmdir /Q/S outsourced\GLFW\obj
del /F/Q/S outsourced\ImGui.vcx%
rmdir /Q/S outsourced\obj

PAUSE