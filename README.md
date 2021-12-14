# IRCamera_OpenCV__QtWidget
OpenCV IRImager C++

## Linux installation | IR Imager Direct-SDK |

## Basic installation

```bash
# cmake
# freeglut3-dev (only if the user wants to compile the OpenGL example)
$ sudo apt-get install cmake freeglut3-dev libusb-1.0-0-dev
```

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


## Interface IR_Camera
![gjkabc](https://github.com/werasaimon/IRCamera_OpenCV__QtWidget/blob/main/image/thermal_img.png)
