#include "labelvideo.h"
#include "Common/IVideoThread.h"
#include <QMouseEvent>
#include <QDebug>

LabelVideo::LabelVideo(QWidget *parent) :
    QLabel(parent)
{

}


//void LabelVideo::mouseDoubleClickEvent(QMouseEvent *event)
//{
//    if ( event->button() == Qt::LeftButton )
//    {
//        qDebug() << "double click";

//        if(mOpenCV_videoCapture != NULL)
//        {
//            if(mOpenCV_videoCapture->getIsRun() && !mOpenCV_videoCapture->getIsWindow())
//            {
////                    mOpenCV_videoCapture->setIsWindow(true);
////                    m_FullWindow = new FullWindow(mOpenCV_videoCapture);
////                    m_FullWindow->show();

//                    qDebug() << "start camera vindow";
//            }
//            else
//            {
//                    qDebug() << "this window is already running";
//            }

//        }
//    }
//}

void LabelVideo::setOpenCV_videoCapture(IVideoThread *newOpenCV_videoCapture)
{
    mOpenCV_videoCapture = newOpenCV_videoCapture;
}


int LabelVideo::IndexCamera() const
{
    return m_IndexCamera;
}

void LabelVideo::setIndexCamera(int newIndexCamera)
{
    m_IndexCamera = newIndexCamera;
}

void LabelVideo::mousePressEvent(QMouseEvent *e)
{
    mMouseCoords.setX(e->pos().x());
    mMouseCoords.setY(e->pos().y());
}

void LabelVideo::mouseReleaseEvent(QMouseEvent *e)
{
    mMouseCoords.setX(e->pos().x());
    mMouseCoords.setY(e->pos().y());
}

void LabelVideo::mouseMoveEvent(QMouseEvent *e)
{
    mMouseCoords.setX(e->pos().x());
    mMouseCoords.setY(e->pos().y());

   //qDebug() << e->pos().x() << "  " << e->pos().y();
}
