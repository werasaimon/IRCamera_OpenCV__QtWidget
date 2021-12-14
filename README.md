# IRCamera_OpenCV__QtWidget
OpenCV IRImager C++

## How to compile OpenCV with Gstreamer [Ubuntu&Windows] - Desktop version
OpenCV and GStremer installed [Install OpenCV | GStremer | : OS-Desktop]
(https://galaktyk.medium.com/how-to-build-opencv-with-gstreamer-b11668fa09c)
![gjkabc](http://documentation.evocortex.com/libirimager2/html/household.png)

## Linux installation | IR Imager Direct-SDK |
#Download the latest version of the libirimager: (https://evocortex.org/downloads)


## Linux installation
```bash
# cmake
# freeglut3-dev (only if the user wants to compile the OpenGL example)
$ sudo apt-get install cmake freeglut3-dev libusb-1.0-0-dev
```

## Basic installation
```bash
# The nodrop option can be passed at system startup by creating the following file:
$ sudo bash -c 'echo "options uvcvideo nodrop=1" > /etc/modprobe.d/uvcvideo.conf'
# This makes the option permanent for the UVC driver (after the next reboot).

# Temporary activation can be achieved with:
$ sudo rmmod uvcvideo; sudo modprobe uvcvideo nodrop=1

#Alternatively, the parameter can be modified via sysfs during runtime:
$ sudo bash -c 'echo -n 1 > /sys/module/uvcvideo/parameters/nodrop'

# Accessing video devices may need the user to be in the system's video group. Checking membership can be done with:
$ groups

# Add the current user to this group, if video is not listed there:
$ sudo usermod -a -G video <username>
```

# Qt5-OpenCV-Raspberry-Pi | Jetson Nano 
Native build of Qt5 creator on Raspberry Pi 4 or Jetson Nano with OpenCV support
## A live camera C++ example on a Raspberry Pi 4 / Jetson Nano with Qt5 GUI using OpenCV <br/> 
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)<br/><br/>
see [Qt5-OpenCV](https://qengineering.eu/install-qt5-with-opencv-on-raspberry-pi-4.html) where this example is explored. 

## Install OpenCV Ubuntu 20.04
[install-OpenCV](https://vitux.com/opencv_ubuntu/)

## Interface IR_Camera
![gjkabc](https://github.com/werasaimon/IRCamera_OpenCV__QtWidget/blob/main/image/thermal_img.png)
