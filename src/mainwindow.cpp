#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Common/IVideoThread.h"
#include <QFileDialog>
#include <QDebug>

// Optris device interfaces
#include <IRDevice.h>
// Optris imager interfaces
#include <IRImager.h>
// Optris logging interface
#include <IRLogger.h>
// Optris raw image file writer
#include <IRFileWriter.h>




namespace
{

    std::string lisen_UDP_gst_JPEG_read_video( int port , int fps)
    {
        std::string gst_video_send = "udpsrc port=" + std::to_string(port) +
                " ! application/x-rtp,media=video,payload=26,clock-rate=90000,encoding-name=JPEG,"
                "framerate=" + std::to_string(fps) + "/1 ! "
                "rtpjpegdepay ! jpegdec ! videoconvert ! appsink max-buffers=1 drop=true sync=0";
        return gst_video_send;
    }

    std::string lisen_UDP_gst_H264_read_video( int port , int fps)
    {
        std::string gst_video_send = "udpsrc port=" + std::to_string(port) +
                " ! application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96,"
                "framerate=" + std::to_string(fps) + "/1 ! "
                "rtph264depay ! decodebin ! videoconvert ! appsink max-buffers=1 drop=true sync=0";
        return gst_video_send;
    }

#if ENABLED_NVIDIA_JETSON
static std::string gst_pipeline_cams[]
{
    "nvarguscamerasrc sensor-id=0 ! video/x-raw(memory:NVMM), width=(int)640, height=(int)480,format=(string)NV12, framerate=(fraction)30/1 !"
    " nvvidconv ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink",

    "nvarguscamerasrc sensor-id=1 ! video/x-raw(memory:NVMM), width=(int)640, height=(int)480,format=(string)NV12, framerate=(fraction)30/1 !"
    " nvvidconv ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink",

    "v4l2src device=/dev/video2 ! video/x-raw, width=(int)640, height=(int)480, format=(string)YUY2, framerate=(fraction)30/1 !"
    " videoconvert ! video/x-raw, width=(int)640, height=(int)480, format=(string)RGB ! videoconvert ! appsink",

    "v4l2src device=/dev/video3 ! video/x-raw, width=(int)640, height=(int)480, format=(string)YUY2, framerate=(fraction)30/1 !"
    " videoconvert ! video/x-raw, width=(int)640, height=(int)480, format=(string)RGB ! videoconvert ! appsink"
};
#else
static std::string gst_pipeline_cams[]
{

    "v4l2src device=/dev/video0 ! video/x-raw, width=(int)640, height=(int)480, format=(string)YUY2, framerate=(fraction)30/1 !"
    " videoconvert ! video/x-raw, width=(int)640, height=(int)480, format=(string)RGB ! videoconvert ! appsink",

    "v4l2src device=/dev/video2 ! video/x-raw, width=(int)640, height=(int)480, format=(string)YUY2, framerate=(fraction)30/1 !"
    " videoconvert ! video/x-raw, width=(int)640, height=(int)480, format=(string)RGB ! videoconvert ! appsink",

    "v4l2src device=/dev/video4 ! video/x-raw, width=(int)640, height=(int)480, format=(string)YUY2, framerate=(fraction)30/1 !"
    " videoconvert ! video/x-raw, width=(int)640, height=(int)480, format=(string)RGB ! videoconvert ! appsink"
};
#endif


static std::string udp_gst_JPEG_send_video( std::string ip , int port , int fps=30)
{
    std::string gst_video_send = "appsrc ! videoconvert ! video/x-raw,format=YUY2,"
                                 "width=(int)640"
                                 ",height=(int)480"
                                 ",framerate=" + std::to_string(fps) + "/1 ! "
                                 "jpegenc ! rtpjpegpay ! "
                                 "udpsink host=" + ip +
                                 " port=" + std::to_string(port) +
                                 " sync=false async=false";
    return gst_video_send;
}

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimerAlarm()));

    //-------------------------------//

    setWindowTitle("IRCamera Thermal");

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

    palette_image.resize(p_w * p_h * 3);
    thermal_data.resize(t_w * t_h);

    //-----------------------------------------------------//

//    // Change thresholds
//    params.minThreshold = 50;
//    params.maxThreshold = 200;

//    // Filter by Area.
//    params.filterByArea = true;
//    params.minArea = 1500;
//    params.maxArea = 50000;

//    // Filter by Circularity
//    params.filterByCircularity = true;
//    params.minCircularity = 0.1;

//    // Filter by Convexity
//    params.filterByConvexity = true;
//    params.minConvexity = 0.2;//0.87;

//    // Filter by Inertia
//    params.filterByInertia = true;
//    params.minInertiaRatio = 0.01;

//    // Filter by Color
//    params.filterByColor = true;
//    params.blobColor = 255;

    //-----------------------------------------------------//

    mLabelVideo = ui->labelVideo;
}

MainWindow::~MainWindow()
{
    ::evo_irimager_terminate();
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
   ::evo_irimager_terminate();
}


void MainWindow::on_pushButton_clicked()
{
    if(!timer->isActive())
    {
       evo_irimager_daemon_is_running();
       timer->start(20); // И запустим таймер
       ui->pushButton->setText("stop CAM");
    }
    else
    {
       timer->stop();
       evo_irimager_daemon_kill();
       ui->pushButton->setText("start CAM");
        ui->labelVideo->clear();
    }
}

void MainWindow::slotTimerAlarm()
{
    ::evo_irimager_set_focusmotor_pos(ui->horizontalSlider_posFocus->value());

    /**
    switch (mTemperatureRangeFlag) {
    case 0: ::evo_irimager_set_temperature_range(0,100); break;
    case 1: ::evo_irimager_set_temperature_range(0,250); break;
    case 2: ::evo_irimager_set_temperature_range(150,900); break;
    }
    **/

    int t_min = ui->spinBox_temperatureMIN->value();
    int t_max = ui->spinBox_temperatureMAX->value();
    ::evo_irimager_set_temperature_range(t_min,t_max);

    int m_min = ui->spinBox_manualMIN->value();
    int m_max = ui->spinBox_manualMAX->value();
    ::evo_irimager_set_palette_manual_temp_range(m_min,m_max);

    //::evo_irimager_set_shutter_mode(ui->checkBox_Shutter->isChecked());
    //::evo_irimager_set_palette_manual_temp_range(150,900);
    //::evo_irimager_set_palette(1);
    //::evo_irimager_set_palette_scale(ui->comboBox_Scale->currentIndex()+1);

    if((err = ::evo_irimager_get_palette_image(&p_w, &p_h, &palette_image[0]))==0)
    {


      ::evo_irimager_get_thermal_image(&t_w,&t_h,&thermal_data[0]);
      unsigned long int mean = 0;
      //--Code for calculation mean temperature of image -----------------
      bool isWhile = true;
      float min_tmp = 0;
      for (int y = 0; y < t_h; y++)
      {
        for (int x = 0; x < t_w; x++)
        {
          int temperature = (int)thermal_data[y*t_w + x];
          mean += temperature;

          if(min_tmp > temperature || isWhile)
          {
              min_tmp = temperature;
              isWhile = false;
          }

        }
      }

      int CelsiusDegree = (mean / (t_h * t_w)) / 10.0 - 100;
      qDebug() << CelsiusDegree;


      //cv::Mat cv_img_thermal(cv::Size(p_w, p_h), CV_8U, &thermal_data[0], cv::Mat::AUTO_STEP);



      //std::cout << p_w << " " << p_h << std::endl;
      //---------------------------------------------
      //--Code for displaying image -----------------
      cv::Mat cv_img(cv::Size(p_w, p_h), CV_8UC3, &palette_image[0], cv::Mat::AUTO_STEP);
      cv::cvtColor(cv_img, cv_img, cv::COLOR_BGR2RGB);
      //cv::flip(cv_img, cv_img, 1);
      cv::resize(cv_img, cv_img, cv::Size(mLabelVideo->width(), mLabelVideo->height()), cv::INTER_CUBIC );
     // cv::imshow("palette image daemon", cv_img);

      //------------------------------------------------------//


      cv::Mat img_gray;
      cv::cvtColor(cv_img, img_gray, cv::COLOR_BGR2GRAY);
      //cv::blur( img_gray, img_gray, cv::Size(3,3) );
      //cv::normalize(img_gray, img_gray, 255, 230, cv::NORM_L2, -1, cv::noArray());


      //-------------------------------------------------------//


      if( ui->checkBox_DetectionThermalArea->isChecked() )
      {
          int thresh = ui->horizontalSlider_Thresh->value();
          int max_thresh = 255;
         // cv::RNG rng(12345);

          cv::Mat threshold_output;
          std::vector<std::vector<cv::Point> > contours;
          std::vector<cv::Vec4i> hierarchy;


          if( ui->checkBox_Blur->isChecked() )
          {
              // Define kernal for erosion and dilation and closing operations
              cv::Mat kernel = cv::Mat(5, 5, CV_8U, cv::Scalar(1,1,1));
              cv::blur( img_gray, img_gray, cv::Size(3,3) );
              cv::erode(img_gray,img_gray,cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
              cv::dilate(img_gray,img_gray,cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
              cv::morphologyEx(img_gray, img_gray, cv::MORPH_CLOSE, kernel);
          }



          /// Detect edges using Threshold
          cv::threshold( img_gray, threshold_output, thresh, max_thresh, cv::THRESH_BINARY );

          /// Find contours
          cv::findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, ui->comboBox_Method->currentIndex() + 1, cv::Point(0, 0) );

          /// Approximate contours to polygons + get bounding rects and circles
          std::vector<std::vector<cv::Point> > contours_poly( contours.size() );
          std::vector<cv::Rect> boundRect( contours.size() );
          std::vector<cv::Point2f>center( contours.size() );
          std::vector<float>radius( contours.size() );

          for( unsigned int i = 0; i < contours.size(); i++ )
          {
              cv::approxPolyDP( cv::Mat(contours[i]), contours_poly[i], 3, true );
              boundRect[i] = boundingRect( cv::Mat(contours_poly[i]) );
              minEnclosingCircle( (cv::Mat)contours_poly[i], center[i], radius[i] );
          }


          /// Draw polygonal contour + bonding rects + circles
          cv::Mat1b mask(cv_img.rows, cv_img.cols, uchar(0));
          //cv::Mat3b dbgRects = cv_img.clone();


          cv::Mat raw_dist( cv_img.size(), CV_32F );
          double minVal, maxVal;
          cv::Point maxDistPt; // inscribed circle center
          minMaxLoc(raw_dist, &minVal, &maxVal, NULL, &maxDistPt);
          minVal = abs(minVal);
          maxVal = abs(maxVal);
          cv::Mat drawing = cv::Mat::zeros( cv_img.size(), CV_8UC3 );

          //==============================================================//

          cv::Mat maskk = cv::Mat::zeros(cv_img.size(),CV_8UC1);
          if(ui->checkBox_InvertMask->isChecked())
          {
              for( size_t i = 0; i < contours.size(); i++ )
              {
                  cv::drawContours( maskk, contours, i, cv::Scalar(255,255,255), cv::FILLED);
              }
          }
          //==============================================================//


          for( unsigned int i = 0; i< contours.size(); i++ )
          {
              int real_w = ((boundRect[i].br().x  - boundRect[i].tl().x));
              int real_h = ((boundRect[i].br().y  - boundRect[i].tl().y));

              if( real_w < 50 && real_h < 50 ) continue;

//              cv::Scalar color = cv::Scalar( 0, 255, 0 );// rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
//              if(ui->checkBox_DrawCountur->isChecked())
//               cv::drawContours( cv_img, contours_poly, i, color, 1, 8, std::vector<cv::Vec4i>(), 0, cv::Point() );

//              if(ui->checkBox_DrawArea->isChecked())
//              cv::rectangle( cv_img, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
              //cv::circle( cv_img, center[i], (int)radius[i], color, 2, 8, 0 );

              //----------------------------------------------------------//

              float unit_w = round(float(mLabelVideo->width())/float(t_w));
              float unit_h = round(float(mLabelVideo->height())/float(t_h));

              int ww = ((boundRect[i].br().x  - boundRect[i].tl().x))/unit_w;
              int hh = ((boundRect[i].br().y  - boundRect[i].tl().y))/unit_h;

              auto pp = boundRect[i].tl();// + (boundRect[i].br() - boundRect[i].tl()) * 0.5f;
             // pp.x = pp.x / unit_w;
              //pp.y = pp.y / unit_h;

              int _x = round(pp.x / unit_w);
              int _y = round(pp.y / unit_h);

              int mean = 0;
              int C_Area = 0;
              int C_Max = 0;
              int C_Min = 0;
              int max_tepe = 0;
              int min_tepe = 0;

              if(_x < t_w && _y < t_h && ww > 0 && hh > 0)
              {
                  bool isInit1 = true;
                  bool isInit2 = true;

                  int num = 0;
                  for (int yy = _y; yy < _y + hh; yy++)
                  {
                      for (int xx = _x; xx < _x + ww; xx++)
                      {

//                          int x = round(xx / unit_w);
//                          int y = round(yy / unit_h);

                          float intersection = (float)pointPolygonTest( contours[i], cv::Point2f((float)(xx*unit_w), (float)(yy*unit_h)), false );
                          if( intersection < 0. )
                          {
                             // drawing.at<cv::Vec3b>(yy*unit_h,xx*unit_w)[0] = (uchar)(255 - abs(raw_dist.at<float>(xx*unit_w,yy*unit_h)) * 255 / minVal);
                          }
                          else if( intersection > 0. )
                          {
                             // drawing.at<cv::Vec3b>(yy*unit_h,xx*unit_w)[2] = (uchar)(255 - raw_dist.at<float>(xx*unit_w,yy*unit_h) * 255 / maxVal);

//                              cv_img.at<cv::Vec3b>(yy*unit_h,xx*unit_w)[0] = 255;
//                              cv_img.at<cv::Vec3b>(yy*unit_h,xx*unit_w)[1] = 255;
//                              cv_img.at<cv::Vec3b>(yy*unit_h,xx*unit_w)[2] = 255;


//                              xx = std::max(0, std::min(xx, t_w));
//                              yy = std::max(0, std::min(yy, t_h));

                              num++;
                              int temperature = (int)thermal_data[(yy)*t_w + (xx)];
                              mean += temperature;

                              //qDebug() << xx << yy << temperature / 10.0 - 100;

                              if(max_tepe < temperature || isInit1)
                              {
                                  max_tepe = temperature;
                                  isInit1 = false;
                              }

                              if(min_tepe > temperature || isInit2)
                              {
                                  min_tepe = temperature;
                                  isInit2 = false;
                              }
                          }
                          else
                          {
//                              drawing.at<cv::Vec3b>(yy*unit_h,xx*unit_w)[0] = 255;
//                              drawing.at<cv::Vec3b>(yy*unit_h,xx*unit_w)[1] = 255;
//                              drawing.at<cv::Vec3b>(yy*unit_h,xx*unit_w)[2] = 255;
                          }
                      }
                  }

                  C_Area = (mean / (num)) / 10.0 - 100;
                  C_Max = (max_tepe) / 10.0 - 100;
                  C_Min = (min_tepe) / 10.0 - 100;
              }


              //----------------------------------------------------------//

              if((CelsiusDegree < C_Area && !(hierarchy[i][3] != -1)) || !ui->checkBox_Analitics->isChecked())
              {

                  cv::Scalar color = cv::Scalar( 0, 255, 0 );// rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
                  if(ui->checkBox_DrawCountur->isChecked())
                  {
                     cv::drawContours( cv_img, contours, i, color, 1, 8, std::vector<cv::Vec4i>(), 0, cv::Point() );
                  }

                  if(ui->checkBox_DrawArea->isChecked())
                  {
                     cv::rectangle( cv_img, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
                     //cv::circle( cv_img, center[i], (int)radius[i], color, 2, 8, 0 );
                  }


                  // Draw white rectangles on mask
                  cv::rectangle(mask, boundRect[i], cv::Scalar(255), CV_FILLED);

        //          // Show rectangles
        //          cv::rectangle(dbgRects, boundRect[i], cv::Scalar(0, 0, 255), 2);

                  cv::putText(cv_img, //target image
                              QString::number(C_Area).toStdString(), //text
                              //"222",
                              boundRect[i].tl() + (boundRect[i].br() - boundRect[i].tl()) * 0.5f, //top-left position
                              cv::FONT_HERSHEY_DUPLEX,
                              0.5,
                              CV_RGB(0, 255, 0), //font color
                              2);


                  cv::putText(cv_img, //target image
                              QString::number(C_Min).toStdString(), //text
                              //"222",
                              (boundRect[i].tl() + (boundRect[i].br() - boundRect[i].tl()) * 0.5f) + cv::Point2i(0,15), //top-left position
                              cv::FONT_HERSHEY_DUPLEX,
                              0.5,
                              CV_RGB(0, 0, 255), //font color
                              2);

              }

              //----------------------------------------------------------//
          }

          // Black initizlied result
           cv::Mat3b result(cv_img.rows, cv_img.cols, cv::Vec3b(0,0,0));
           cv_img.copyTo(result, mask);

           if(ui->checkBox_InvertMask->isChecked())
           {
               // Black initizlied result
               cv::Mat3b result(cv_img.rows, cv_img.cols, cv::Vec3b(0,0,0));
               cv_img.copyTo(result, (ui->checkBox_DrawArea->isChecked()) ? mask : maskk);
               cv_img = result;
           }

          /**/

      }



      if(ui->checkBox_PoinThermal->isChecked())
      {

          auto Pmouse = mLabelVideo->mMouseCoords;

          float unit_w = round(float(mLabelVideo->width())/float(t_w));//(t_w / (mLabelVideo->width()/mLabelVideo->height())) * 0.5;
          float unit_h = round(float(mLabelVideo->height())/float(t_h));//(t_h / (mLabelVideo->width()/mLabelVideo->height())) * 0.5;

          cv::Point point(Pmouse.x(), Pmouse.y());

          int x = round(Pmouse.x() / unit_w);
          int y = round(Pmouse.y() / unit_h);

         // std::cout<< "test: "  << unit_w <<  " " << unit_h << std::endl;
         // std::cout << cv::Point(x,y) << " (" << Pmouse.x() / 11.f << "," << Pmouse.y() / 5.f << ")" << std::endl;

//          int r = 1;
//          circle(cv_img, point , r, cv::Scalar(0, 255, 100), 5);



          int temperature = (int)thermal_data[y*t_w + x];
          temperature = (temperature) / 10.0 - 100;

          cv::putText(cv_img, //target image
                      QString::number(temperature).toStdString(), //text
                      //"222",
                      point + cv::Point(5,-10), //top-left position
                      cv::FONT_HERSHEY_DUPLEX,
                      0.5,
                      CV_RGB(0, 255, 0), //font color
                      2);

          cv::drawMarker(cv_img, point, cv::Scalar(255, 255, 255), cv::MARKER_CROSS, 15, 1);
      }


      //-------------------------------------------------------//

      int C = (mean / (t_h * t_w)) / 10.0 - 100;
      auto img = IVideoThread::cvMatToQPixmap(cv_img, false, QString::number(C) + " °C");
      mLabelVideo->setPixmap(img);

      //-------------------------------------------------------//
    }
    else
    {
      std::cerr << "failed evo_irimager_get_thermal_palette_image: " << err << std::endl;
    }
}



void MainWindow::on_comboBox_Mode_currentIndexChanged(int index)
{
    ::evo_irimager_set_palette(1+index);
}


void MainWindow::on_comboBox_Scale_currentIndexChanged(int index)
{
    ::evo_irimager_set_palette_scale(index+1);
}


void MainWindow::on_checkBox_Shutter_toggled(bool checked)
{
    ::evo_irimager_set_shutter_mode(checked);
}


void MainWindow::on_actionimport_XML_settings_triggered()
{
    auto file1Name = QFileDialog::getOpenFileName(this, tr("Open XML File 1"), "/home", tr("XML Files (*.xml)"));

    if (QFile::copy(file1Name, "ccgenericc2.xml"))
    {
        std::cout << "YES COPY FILE  ccgenericc.xml" << std::endl;
    }

    ::evo_irimager_terminate();

    if(::evo_irimager_usb_init("ccgenericc2.xml", 0, 0) != 0)
    {
        std::cout << "no file : " << file1Name.toStdString().c_str() << std::endl;
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

    palette_image.resize(p_w * p_h * 3);
    thermal_data.resize(t_w * t_h);

    float pos_pov;
    ::evo_irimager_get_focusmotor_pos(&pos_pov);
    ui->horizontalSlider_posFocus->setValue(pos_pov);
}





void MainWindow::on_pushButton_2_clicked()
{

}

