#include "IVideoThread.h"
#include <QImage>
#include <QPainter>
#include <QDebug>

static std::vector<unsigned char> palette_image;
static std::vector<unsigned short> thermal_data;

IVideoThread::IVideoThread(QObject *parent, QString _Name)
    :QThread { parent } ,
    isRun(false) ,
    isWindow(false),
    m_Name(_Name)

{
    //--------------------------------------//
    if (QFile::exists("ccgenericc2.xml"))
    {
        QFile::remove("ccgenericc2.xml");
        std::cout << "DELETE  ccgenericc.xml" << std::endl;
    }
    if (QFile::copy(":/resources/genericc.xml", "ccgenericc2.xml"))
    {
        std::cout << "YES COPY FILE  ccgenericc.xml" << std::endl;
    }
    //--------------------------------------//

    if(::evo_irimager_usb_init("ccgenericc2.xml", 0, 0) != 0)
    {
        std::cout << "no file : " << "ccgenericc2.xml" << std::endl;
        return;
    }

    if((err = ::evo_irimager_get_palette_image_size(&p_w, &p_h)) != 0)
    {
      std::cerr << "error on evo_irimager_get_palette_image_size: " << err << std::endl;
      exit(-1);
    }

    if((err = ::evo_irimager_get_thermal_image_size(&t_w, &t_h)) != 0)
    {
      std::cerr << "error on evo_irimager_get_palette_image_size: " << err << std::endl;
      exit(-1);
    }
}

IVideoThread::~IVideoThread()
{
   isRun = false;
   exit(0);

   mVideoCapture.release();
   mVideoWriter.release();

   ::evo_irimager_terminate();
   //terminate();
}

void IVideoThread::run()
{

    if(mVideoWriter.isOpened())
    {
      std::cout  << " \n ------------------------------- \n "
                    "start writer cpture video  |OK| \n"
                    " \n ------------------------------- \n ";
    }





    do
    {
      //::evo_irimager_set_focusmotor_pos(pos+=0.1);
      //::evo_irimager_set_temperature_range(0,250);
      //::evo_irimager_set_shutter_mode(3);
      //::evo_irimager_set_palette_manual_temp_range(150,900);
      //::evo_irimager_set_palette(1);
      //::evo_irimager_set_palette_scale(3);


        m_mutex.lock();

        if((err = ::evo_irimager_get_thermal_palette_image(80, 80, &thermal_data[0],
                                                           80, 80, &palette_image[0]))==0)
        {
            std::cerr << "failed evo_irimager_get_thermal_palette_image: " << err << std::endl;
            //      }

             QThread::msleep(35);
        }
        m_mutex.unlock();

//        //---------------------------------------------
//        //--Code for displaying image -----------------
//        cv::Mat cv_img(cv::Size(p_w, p_h), CV_8UC3, &palette_image[0], cv::Mat::AUTO_STEP);
//        cv::cvtColor(cv_img, mFrame, cv::COLOR_BGR2RGB);

//        if(!mFrame.empty())
//        {
//          if(mVideoWriter.isOpened())
//          {
//             mVideoWriter << mFrame;
//          }

//          mPixmap = cvMatToQPixmap(mFrame,isText,m_Name);
//          emit newPixmapCapture();

//        }

//        //cv::resize(cv_img, cv_img, cv::Size(500, 500), cv::INTER_LINEAR);
//        //cv::imshow("palette image daemon", cv_img);
//        //---------------------------------------------
//        //---------------------------------------------
//      }
//      else
//      {
//        std::cerr << "failed evo_irimager_get_thermal_palette_image: " << err << std::endl;
//      }

    } while(isRun);//cvGetWindowHandle("palette image daemon"));

    /**
     while (isRun)
     {
        if(mVideoCapture.isOpened())
        {
           mVideoCapture >> mFrame;
           if(!mFrame.empty())
           {
             if(mVideoWriter.isOpened())
             {
                mVideoWriter << mFrame;
             }

             mPixmap = cvMatToQPixmap(mFrame,isText,m_Name);
             emit newPixmapCapture();

           }
        }
     }
     **/
}

bool IVideoThread::getIsText() const
{
    return isText;
}

void IVideoThread::setIsText(bool newIsText)
{
    isText = newIsText;
}

const QString &IVideoThread::Name() const
{
    return m_Name;
}

void IVideoThread::setName(const QString &_str)
{
    m_Name = _str;
}

bool IVideoThread::getIsWindow() const
{
    return isWindow;
}

void IVideoThread::setIsWindow(bool newIsWindow)
{
    isWindow = newIsWindow;
}

bool IVideoThread::getIsRun() const
{
    return isRun;
}

void IVideoThread::setIsRun(bool newIsRun)
{
    isRun = newIsRun;
}

cv::VideoCapture &IVideoThread::VideoCapture()
{
    return mVideoCapture;
}

cv::VideoWriter &IVideoThread::VideoWriter()
{
    return mVideoWriter;
}

QImage IVideoThread::cvMatToQImage( const cv::Mat &inMat , bool isText , QString _str)
{
  switch ( inMat.type() )
  {
     // 8-bit, 4 channel
     case CV_8UC4:
     {
        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_ARGB32 );

        if( isText )
        {
            QPainter painter(&image);
            //painter.setBrush(QBrush(Qt::green));
            //painter.fillRect(QRectF(0,0,400,300),Qt::green);
            //painter.fillRect(QRectF(100,100,200,100),Qt::white);

            QFont* FontText = new QFont("Serif", 5, QFont::Normal);

            painter.setPen(QPen(Qt::red));
            painter.setFont(*FontText);
            painter.drawText(2,4,_str);

            delete FontText;

        }

        return image;
     }

     // 8-bit, 3 channel
     case CV_8UC3:
     {
        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_RGB888 );

       // image = image.mirrored(true,false);

        if( isText )
        {
            QPainter painter(&image);
            //painter.setBrush(QBrush(Qt::green));
            //painter.fillRect(QRectF(0,0,400,300),Qt::green);
            //painter.fillRect(QRectF(100,100,200,100),Qt::white);

            QFont* FontText = new QFont("Arial", 20, QFont::Normal);

            painter.setPen(QPen(Qt::green));
            painter.setFont(*FontText);
            painter.drawText(20,40,_str);


            delete FontText;

        }

        return image.rgbSwapped();
     }

     // 8-bit, 1 channel
     case CV_8UC1:
     {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_Grayscale8 );
#else
        static QVector<QRgb>  sColorTable;

        // only create our color table the first time
        if ( sColorTable.isEmpty() )
        {
           sColorTable.resize( 256 );

           for ( int i = 0; i < 256; ++i )
           {
              sColorTable[i] = qRgb( i, i, i );
           }
        }

        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_Indexed8 );

        image.setColorTable( sColorTable );
#endif

        if( isText )
        {
            QPainter painter(&image);
            //painter.setBrush(QBrush(Qt::green));
            //painter.fillRect(QRectF(0,0,400,300),Qt::green);
            //painter.fillRect(QRectF(100,100,200,100),Qt::white);

            QFont* FontText = new QFont("Serif", 20, QFont::Normal);

            painter.setPen(QPen(Qt::green));
            painter.setFont(*FontText);
            painter.drawText(20,40,_str);

            delete FontText;

        }

        return image;
     }

     default:
        qWarning() << "ASM::cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
        break;
  }

  return QImage();
}
QPixmap IVideoThread::cvMatToQPixmap(const cv::Mat &inMat , bool isText, QString _str)
{
  return QPixmap::fromImage( cvMatToQImage( inMat , isText , _str) );
}
