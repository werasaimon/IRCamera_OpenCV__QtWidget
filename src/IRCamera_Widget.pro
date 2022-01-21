QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11


INCLUDEPATH += /usr/include/opencv4
INCLUDEPATH += /usr/include/libirimager
DEPENDPATH += /usr/include/opencv4

#set package support if disabled
QT_CONFIG -= no-pkg-config

#link opencv4 package
CONFIG += link_pkgconfig
PKGCONFIG += opencv4

LIBS += -L/usr/lib \
        -lopencv_core \
        -lopencv_highgui \
        -lopencv_imgproc \
        -lopencv_photo \
        -lopencv_imgcodecs \
        -lopencv_video \
        -lopencv_videoio \
        -lopencv_features2d \
#        -lopencv_tracking \
        -lopencv_dnn \
#        -L/usr/local/cuda
#        -lcuda \
#        -lcudar \
         -lirdirectsdk \
         -ludev \
         -lpthread \
         -lusb-1.0

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Common/IVideoThread.cpp \
    Common/blockreader.cpp \
    Common/blockwriter.cpp \
    fullwindow.cpp \
    labelvideo.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    Common/IVideoThread.h \
    Common/blockreader.h \
    Common/blockwriter.h \
    Common/scommand.hpp \
    fullwindow.h \
    labelvideo.h \
    mainwindow.h

FORMS += \
    fullwindow.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
