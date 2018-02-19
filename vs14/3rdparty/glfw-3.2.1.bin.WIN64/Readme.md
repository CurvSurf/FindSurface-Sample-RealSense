# FindSurface-Samples / RealSense / GLFW / Readme.md
**Curv*Surf* FindSurface™ SDK** Samples - RealSense

Download GLFW windows binaries at [here](http://www.glfw.org/download.html).

#### Windows

Unzip the file in the 3rdparty directory.      
Make sure that the files are located as follows:

- For 32-bit platforms:
	- vs14\3rdparty\glfw-3.2.1.bin.WIN32\lib-vc2015\glfw3.dll
	- vs14\3rdparty\glfw-3.2.1.bin.WIN32\include\GLFW\glfw3.h
	- vs14\3rdparty\glfw-3.2.1.bin.WIN32\lib-vc2015\glfw3dll.lib

- For 64-bit platforms:
	- vs14\3rdparty\glfw-3.2.1.bin.WIN64\lib-vc2015\glfw3.dll
	- vs14\3rdparty\glfw-3.2.1.bin.WIN32\include\GLFW\glfw3.h
	- vs14\3rdparty\glfw-3.2.1.bin.WIN32\lib-vc2015\glfw3

Or modify the library paths in SimpleGUI project settings (SimpleGUI.vcxproj) into the corresponding paths that your files are located at.

This project uses a post-build event that copies required .dll files to $(OutDir), so you also have to modify it if you modify the library paths.

If you downloaded a different version of GLFW, the library paths should be appropriately modified to locate the library files.

#### Linux

Follow the instructions in [here](https://github.com/CurvSurf/FindSurface-Sample-RealSense#requirements) to install the library.
