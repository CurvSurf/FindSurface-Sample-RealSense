# FindSurface-Samples / FindSurface_RealSense_Demo / Readme.md
**Curv*Surf* FindSurface™ SDK** Sample - FindSurface_RealSense_Demo

This is a pre-release version and we are working on this.

Overview
--------

FindSurface SDK sample that supports Intel RealSense D435 device.


**The sample only runs with our [FindSurface SDK library files](https://developers.curvsurf.com/downloads.jsp) (FindSurface.dll, etc.).**

**You must either [request a free trial](http://developers.curvsurf.com/licenses.jsp) or [purchase a license](https://developers.curvsurf.com/licenses.jsp) to activate the library files.**

**Please read and follow the instructions [here](https://developers.curvsurf.com/documentation.jsp) > GUIDES > How-To > License Activation, before you activate the SDK license.**


Requirements
--------

### Windows

This sample requires GLEW, GLFW, librealsense libraries.

You can download each of them at:
- GLEW: [glew sourceforge](http://glew.sourceforge.net/)
- GLFW: [official website](http://www.glfw.org/download.html)
- librealsense2: [librealsense release(github)](https://github.com/IntelRealSense/librealsense)
- FreeType2: [official website](https://www.freetype.org/download.html)

To build this sample, you need:

- Visual Studio 2015 or higher

To run this sample, you need:

- OpenGL 4.3+ compatible graphics card (dedicated or integrated)
- Intel RealSense Device (D435)


### Linux

This sample requires GLEW, GLFW, librealsense and FreeType2 libraries.

We recommend you to build the sample in Ubuntu 16.04 or higher version   
(or you have to manually build GLEW and GLFW libraries instead of using `apt-get install`).

[How to install librealsense in Linux](https://github.com/IntelRealSense/librealsense/blob/v1.12.1/doc/installation.md#video4linux-backend)   

You have to follow all the instructions (including Video4Linux backend) to successfully set up the library.

Additionally, you need to install GLFW and GLEW if you have not installed them yet in your system.

```SH
sudo apt-get install libglfw3-dev libglew-dev
```


Contact
-------

Send an email to support@curvsurf.com to contact our support team, if you have any question to ask.
