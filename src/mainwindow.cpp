#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Common/IVideoThread.h"
#include <QFileDialog>

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


    setWindowTitle("IRCamera Thermal");

//    //--------------------------------------//
//    if (QFile::exists("ccgenericc2.xml"))
//    {
//        QFile::remove("ccgenericc2.xml");
//        std::cout << "DELETE  ccgenericc.xml" << std::endl;
//    }
//    if (QFile::copy(":/resources/genericc.xml", "ccgenericc2.xml"))
//    {
//        std::cout << "YES COPY FILE  ccgenericc.xml" << std::endl;
//    }
//    //--------------------------------------//

//    if(::evo_irimager_usb_init("ccgenericc2.xml", 0, 0) != 0)
//    {
//        std::cout << "no file : " << "ccgenericc2.xml" << std::endl;
//        return;
//    }

//    if((err = ::evo_irimager_get_palette_image_size(&p_w, &p_h)) != 0)
//    {
//      std::cerr << "error on evo_irimager_get_palette_image_size: " << err << std::endl;
//      exit(-1);
//    }

//    if((err = ::evo_irimager_get_thermal_image_size(&t_w, &t_h)) != 0)
//    {
//      std::cerr << "error on evo_irimager_get_palette_image_size: " << err << std::endl;
//      exit(-1);
//    }

//    palette_image.resize(p_w * p_h * 3);
//    thermal_data.resize(t_w * t_h);
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
      //std::cout << p_w << " " << p_h << std::endl;
      //---------------------------------------------
      //--Code for displaying image -----------------
      cv::Mat cv_img(cv::Size(p_w, p_h), CV_8UC3, &palette_image[0], cv::Mat::AUTO_STEP);
      cv::cvtColor(cv_img, cv_img, cv::COLOR_BGR2RGB);
      cv::resize(cv_img, cv_img, cv::Size(ui->labelVideo->width(), ui->labelVideo->height()), cv::INTER_LINEAR);
     // cv::imshow("palette image daemon", cv_img);

      int C = (mean / (t_h * t_w)) / 10.0 - 100;
      auto img = IVideoThread::cvMatToQPixmap(cv_img,true, QString::number(C) + " °C");
      ui->labelVideo->setPixmap(img);
      //---------------------------------------------
      //---------------------------------------------
    }
    else
    {
      std::cerr << "failed evo_irimager_get_thermal_palette_image: " << err << std::endl;
    }
}


void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    mTemperatureRangeFlag = index;
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

