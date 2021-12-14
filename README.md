# IRCamera_OpenCV__QtWidget
OpenCV IRImager C++

##Linux installation | IR Imager Direct-SDK |

```bash
# clone imath from GitHub
$ sudo bash -c 'echo "options uvcvideo nodrop=1" > /etc/modprobe.d/uvcvideo.conf'
$ sudo rmmod uvcvideo; sudo modprobe uvcvideo nodrop=1
$ sudo bash -c 'echo -n 1 > /sys/module/uvcvideo/parameters/nodrop'
$ groups
$ sudo usermod -a -G video <username>
```


## Interface IR_Camera
![gjkabc](https://github.com/werasaimon/IRCamera_OpenCV__QtWidget/blob/main/image/thermal_img.png)
