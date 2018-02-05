# FindSurface-Samples / RealSense / libFindSurface / Readme.md
**Curv*Surf* FindSurface™ SDK** Samples - RealSense

1. Download FindSurface SDK library at [here](https://developers.curvsurf.com/downloads.jsp)

2. Unzip and put the contents to this directory.      
Make sure that the files are located as follows:

- For 32-bit platforms:
	- vs14\libFindSurface\include\FindSurface.h
	- vs14\libFindSurface\lib\x86\FindSurface.dll
	- vs14\libFindSurface\lib\x86\FindSurface.lib

- For 64-bit platforms:
	- vs14\libFindSurface\include\FindSurface.h
	- vs14\libFindSurface\lib\x64\FindSurface.dll
	- vs14\libFindSurface\lib\x64\FindSurface.lib

Or change the library paths in SimpleGUI project settings (SimpleGUI.vcxproj) into the corresponding paths that your files are located.

This project uses a post-build event that copies required .dll files to $(OutDir), so you also have to modify it if you modify the library paths.