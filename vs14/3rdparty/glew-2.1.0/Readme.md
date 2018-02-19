# FindSurface-Samples / RealSense / glew / Readme.md
**Curv*Surf* FindSurface™ SDK** Samples - RealSense

Download GLEW windows binaries at [here](http://glew.sourceforge.net/).

#### Windows

Unzip the file in the 3rdparty directory.      
Make sure that the files are located as follows:

- For 32-bit platforms:
	- vs14\3rdparty\glew-2.1.0\bin\Release\Win32\glew32.dll
	- vs14\3rdparty\glew-2.1.0\include\GL\glew.h
	- vs14\3rdparty\glew-2.1.0\include\GL\glxew.h
	- vs14\3rdparty\glew-2.1.0\include\GL\wglew.h
	- vs14\3rdparty\glew-2.1.0\lib\Release\Win32\glew32.lib

- For 64-bit platforms:
	- vs14\3rdparty\glew-2.1.0\bin\Release\x64\glew32.dll
	- vs14\3rdparty\glew-2.1.0\include\GL\glew.h
	- vs14\3rdparty\glew-2.1.0\include\GL\glxew.h
	- vs14\3rdparty\glew-2.1.0\include\GL\wglew.h
	- vs14\3rdparty\glew-2.1.0\lib\Release\x64\glew32.lib

Or modify the library paths in SimpleGUI project settings (SimpleGUI.vcxproj) into the corresponding paths that your files are located at.

This project uses a post-build event that copies required .dll files to $(OutDir), so you also have to modify it if you modify the library paths.

If you downloaded a different version of glew, the library paths should be appropriately modified to locate the library files.

#### Linux

Follow the instructions in [here](https://github.com/CurvSurf/FindSurface-Sample-RealSense#requirements) to install the library.