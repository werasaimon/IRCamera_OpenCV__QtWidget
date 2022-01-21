#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Common/IVideoThread.h"
#include <QFileDialog>

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
       ui->pushButton->setText("start CAM");
    }
    else
    {
       timer->stop();
       evo_irimager_daemon_kill();
       ui->pushButton->setText("stop CAM");
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
      for (int y = 0; y < t_h; y++)
      {
        for (int x = 0; x < t_w; x++)
        {
          mean += thermal_data[y*t_w + x];
        }
      }


      //cv::Mat cv_img_thermal(cv::Size(p_w, p_h), CV_8U, &thermal_data[0], cv::Mat::AUTO_STEP);



      //std::cout << p_w << " " << p_h << std::endl;
      //---------------------------------------------
      //--Code for displaying image -----------------
      cv::Mat cv_img(cv::Size(p_w, p_h), CV_8UC3, &palette_image[0], cv::Mat::AUTO_STEP);
      cv::cvtColor(cv_img, cv_img, cv::COLOR_BGR2RGB);
      cv::flip(cv_img, cv_img, 1);
      cv::resize(cv_img, cv_img, cv::Size(ui->labelVideo->width(), ui->labelVideo->height()), cv::INTER_CUBIC );
     // cv::imshow("palette image daemon", cv_img);

      //------------------------------------------------------//




      cv::Mat img_gray;
      cv::cvtColor(cv_img, img_gray, cv::COLOR_BGR2GRAY);
      //cv::blur( img_gray, img_gray, cv::Size(3,3) );
      //cv::normalize(img_gray, img_gray, 255, 230, cv::NORM_L2, -1, cv::noArray());


      /**

      cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params);

      std::vector<cv::KeyPoint> keypoints;
      detector->detect( img_gray , keypoints);

      //std::cout << keypoints.size() << std::endl;

      cv::drawKeypoints( cv_img, keypoints, cv_img,
                         cv::Scalar(0,0,255),
                         cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS );


      for (unsigned int i = 0; i < keypoints.size(); ++i)
      {
          cv::Point2f p = keypoints[i].pt;
          float size = keypoints[i].size;

          // our rectangle...
          cv::Rect rect(p.x - size * 0.5, p.y - size * 0.5, size, size);
          cv::rectangle(cv_img,rect,cv::Scalar(255,0,0),1,8,0);


          mean = 0;

          int ww = ((size) * 0.5)/8.f;
          int hh = ((size) * 0.5)/6.f;

          auto pp = p - cv::Point2f(size * 0.5,size * 0.5);
          pp.x = pp.x / 8;
          pp.y = pp.y / 6;

          int C = 0;
          if(pp.x < 80 && pp.x > 0 &&
             pp.y < 80 && pp.y > 0 &&
             ww > 0 && hh > 0)
          {
             for (int yy = pp.y; yy < pp.y + hh; yy++)
             {
               for (int xx = pp.x; xx < pp.x + ww; xx++)
               {
                 mean += thermal_data[yy*ww + xx];
               }
             }

             C = (mean / (ww * hh)) / 10.0 - 100;
          }

          //p.x = p.x - 20;
          cv::putText(cv_img, //target image
                      QString::number(C).toStdString(), //text
                      //"255", //text
                      p, //top-left position
                      cv::FONT_HERSHEY_DUPLEX,
                      0.5,
                      CV_RGB(0, 255, 0), //font color
                      2);
      }

      **/

      //-------------------------------------------------------//

      /**/


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
          //cv::Mat drawing = cv::Mat::zeros( threshold_output.size(), CV_8UC3 );

          for( unsigned int i = 0; i< contours.size(); i++ )
          {
              int real_w = ((boundRect[i].br().x  - boundRect[i].tl().x));
              int real_h = ((boundRect[i].br().y  - boundRect[i].tl().y));

              if( real_w < 50 && real_h < 50 ) continue;

              cv::Scalar color = cv::Scalar( 0, 255, 0 );// rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
              if(ui->checkBox_DrawCountur->isChecked())
               cv::drawContours( cv_img, contours_poly, i, color, 1, 8, std::vector<cv::Vec4i>(), 0, cv::Point() );

              if(ui->checkBox_DrawArea->isChecked())
              cv::rectangle( cv_img, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
              //cv::circle( cv_img, center[i], (int)radius[i], color, 2, 8, 0 );

              //----------------------------------------------------------//

              int unit_w = ui->labelVideo->width() / t_w;
              int unit_h = ui->labelVideo->height() / t_h;

              int ww = ((boundRect[i].br().x  - boundRect[i].tl().x))/unit_w;
              int hh = ((boundRect[i].br().y  - boundRect[i].tl().y))/unit_h;

              auto pp = boundRect[i].tl();// + (boundRect[i].br() - boundRect[i].tl()) * 0.5f;
              pp.x = pp.x / unit_w;
              pp.y = pp.y / unit_h;

              int mean = 0;
              int C_Area = 0;
              int C_Max = 0;
              int max_tmp  = -1000;
              if(pp.x < t_w && pp.y < t_h && ww > 0 && hh > 0)
              {
                  for (int yy = pp.y; yy < pp.y + hh; yy++)
                  {
                      for (int xx = pp.x; xx < pp.x + ww; xx++)
                      {
                          int temperature = (int)thermal_data[yy*ww + xx];
                          mean += temperature;

                          if(max_tmp < temperature || max_tmp == -1000)
                          {
                              max_tmp = temperature;
                          }

                      }
                  }
                  C_Area = (mean / (ww * hh)) / 10.0 - 100;
                  C_Max = (max_tmp) / 10.0 - 100;
              }


              //----------------------------------------------------------//

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
                          QString::number(C_Max).toStdString(), //text
                          //"222",
                          (boundRect[i].tl() + (boundRect[i].br() - boundRect[i].tl()) * 0.5f) + cv::Point2i(0,15), //top-left position
                          cv::FONT_HERSHEY_DUPLEX,
                          0.5,
                          CV_RGB(0, 0, 255), //font color
                          2);

              //----------------------------------------------------------//
          }

          // Black initizlied result
           cv::Mat3b result(cv_img.rows, cv_img.cols, cv::Vec3b(0,0,0));
           cv_img.copyTo(result, mask);

           if(ui->checkBox_InvertMask->isChecked())
           {
               cv_img = result;
           }

          /**/

      }


      //-------------------------------------------------------//

      int C = (mean / (t_h * t_w)) / 10.0 - 100;
      auto img = IVideoThread::cvMatToQPixmap(cv_img, false, QString::number(C) + " °C");
      ui->labelVideo->setPixmap(img);

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

