

![gjkabc](https://leotronics.eu/templates/rt_horizon/custom/images/leotronics/logo-leotronics-black-10.jpg) 
# LEOTRONICS https://leotronics.eu/en/ 

# OpenCV IRImager C++ | IR Imager Direct-SDK | QT
### Linux installation | IR Imager Direct-SDK |


Instrction insalation libirimager | Linux / Windows |
http://documentation.evocortex.com/libirimager2/html/Installation.html

#Download the latest version of the libirimager: (https://evocortex.org/downloads)

## Image IRCamera Photo
![gjkabc](http://documentation.evocortex.com/libirimager2/html/household.png)


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

#Do the basic installation as follows:
#Download the latest version of the libirimager: DOWNLOAD LINK
```

## Basic Settin IRImageDirect
```bash
$ sudo ir_download_calibration

#Everything is installed to /usr/share/libirimager/cali.
#If you like to specify a user-defined directory for the calibration files, append the following option:
$ ir_download_calibration -t /tmp
```

# Qt5-OpenCV-Raspberry-Pi | Jetson Nano 
Native build of Qt5 creator on Raspberry Pi 4 or Jetson Nano with OpenCV support
## A live camera C++ example on a Raspberry Pi 4 / Jetson Nano with Qt5 GUI using OpenCV <br/> 
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)<br/><br/>
see [Qt5-OpenCV](https://qengineering.eu/install-qt5-with-opencv-on-raspberry-pi-4.html) where this example is explored. 

## Install OpenCV Ubuntu 20.04
[install-OpenCV](https://vitux.com/opencv_ubuntu/)

## Qt-Interface IR_Camera
![gjkabc](https://github.com/werasaimon/IRCamera_OpenCV__QtWidget/blob/main/image/thermal_img.png)
