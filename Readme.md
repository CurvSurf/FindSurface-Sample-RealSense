# FindSurface-Samples / FindSurface_RealSense_Demo / Readme.md
**Curv*Surf* FindSurface™ SDK** Sample - FindSurface_RealSense_Demo



Overview
--------

This sample demonstrates how FindSurface works with Intel RealSense devices (R200, ZR300).

This sample program allows users to pick an object to make a rough 3D snapshot of the object.

Intel RealSense devices (R200 or ZR300) is required to run the sample program.

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
- librealsense: [librealsense legacy release(github)](https://github.com/IntelRealSense/librealsense/tree/v1.12.1)

To build this sample, you need:

- Visual Studio 2015 or higher version (for C++11 features such as lambda function)

To run this sample, you need:

- OpenGL 4.3+ support graphics card (disrete or integrated)
- Intel RealSense Device (ZR300 or R200)


### Linux

This sample requires GLEW, GLFW, librealsense libraries.

We recommend you to build the sample in Ubuntu 16.04 or higher version   
(or you have to manually build GLEW and GLFW libraries instead of using `apt-get install`).

[How to install librealsense in Linux](https://github.com/IntelRealSense/librealsense/blob/v1.12.1/doc/installation.md#video4linux-backend)   

You have to follow all the instructions (including Video4Linux backend) to successfully set up the library.

Additionally, you need to install GLFW and GLEW if you have not installed them yet in your system.

```SH
sudo apt-get install libglfw3-dev libglew-dev
```

### Before building the sample

Download FindSurface SDK library files at [our developer website](https://developers.curvsurf.com/downloads.jsp).

Make sure you have downloaded and installed the 3rdparty libraries mentioned above.


Contact
-------

Send an email to support@curvsurf.com to contact our support team, if you have any question to ask.
