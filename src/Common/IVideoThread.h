#ifndef IVIDEOTHREAD_H
#define IVIDEOTHREAD_H

#include <QPixmap>
#include <QImage>
#include <QThread>
#include <QString>
#include <opencv4/opencv2/opencv.hpp>
#include "scommand.hpp"

#include <QFile>
#include <QMutex>

#include <iostream>
#include <memory>
#include <opencv4/opencv2/highgui/highgui_c.h>
//--Code for displaying image -----------------
#include <opencv4/opencv2/opencv.hpp>
#include "libirimager/direct_binding.h"
//---------------------------------------------


#define ID_CAMERA 0

class IVideoThread : public QThread
{
    Q_OBJECT
public:
    IVideoThread(QObject *parent = nullptr , QString _Name = 0);
    ~IVideoThread();

    QPixmap pixmap() const
    {
        return mPixmap;
    }
    cv::VideoCapture &VideoCapture();
    cv::VideoWriter &VideoWriter();

    void setIsRun(bool newIsRun);
    bool getIsRun() const;

    bool getIsWindow() const;
    void setIsWindow(bool newIsWindow);

    const QString &Name() const;
    void setName(const QString& _str);

    bool getIsText() const;
    void setIsText(bool newIsText);

signals:
    void newPixmapCapture(); //capture a frame
protected:
    void run() override;
private:
    QPixmap mPixmap;              //Qt image
    cv::Mat mFrame;               //OpenCV image
    cv::VideoCapture mVideoCapture;   //video capture
    cv::VideoWriter  mVideoWriter;   //video write
    bool isRun;
    bool isWindow;
    bool isText;

    QString m_Name;

public:

    static QImage cvMatToQImage(const cv::Mat &inMat, bool isText , QString _str);
    static QPixmap cvMatToQPixmap(const cv::Mat &inMat, bool isText , QString _str);

    //------------------------//

    int err;
    int p_w;
    int p_h;
    int t_w;
    int t_h;



    QMutex	m_mutex;
    //------------------------//
};

#endif // IVIDEOTHREAD_H
