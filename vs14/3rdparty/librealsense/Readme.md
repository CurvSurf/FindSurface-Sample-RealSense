# FindSurface-Samples / RealSense / librealsense / Readme.md
**Curv*Surf* FindSurface™ SDK** Samples - RealSense

1. Download librealsense the latest legacy release at [here](https://github.com/IntelRealSense/librealsense/tree/v1.12.1)

2. You have to build the library by yourself.

3. Put the library files to this directory.      
Make sure that the files are located as follows:

- For 32-bit platforms:
	- vs14\3rdparty\librealsense\includes\rs.h
	- vs14\3rdparty\librealsense\includes\rs.hpp
	- vs14\3rdparty\librealsense\includes\rscore.hpp
	- vs14\3rdparty\librealsense\includes\rsutil.h
	- vs14\3rdparty\librealsense\libs\x86\realsense.lib
	- vs14\3rdparty\librealsense\libs\x86\realsense.dll

- For 64-bit platforms:
	- vs14\3rdparty\librealsense\includes\rs.h
	- vs14\3rdparty\librealsense\includes\rs.hpp
	- vs14\3rdparty\librealsense\includes\rscore.hpp
	- vs14\3rdparty\librealsense\includes\rsutil.h
	- vs14\3rdparty\librealsense\libs\x64\realsense.lib
	- vs14\3rdparty\librealsense\libs\x64\realsense.dll

Or change the library paths in SimpleGUI project settings (SimpleGUI.vcxproj) into the corresponding paths that your files are located.

This project uses a post-build event that copies required .dll files to $(OutDir), so you also have to modify it if you modify the library paths.