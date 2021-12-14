#include "labelvideo.h"
#include "Common/IVideoThread.h"
#include <QMouseEvent>
#include <QDebug>

LabelVideo::LabelVideo(QWidget *parent) :
    QLabel(parent)
{

}

void LabelVideo::setOpenCV_videoCapture(IVideoThread *newOpenCV_videoCapture)
{
    mOpenCV_videoCapture = newOpenCV_videoCapture;
}


