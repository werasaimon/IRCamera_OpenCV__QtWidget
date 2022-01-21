#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <QCloseEvent>
#include "labelvideo.h"

#include <opencv4/opencv2/highgui/highgui_c.h>
#include <opencv4/opencv2/opencv.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void closeEvent(QCloseEvent *event);

private slots:
    void on_pushButton_clicked();
    void slotTimerAlarm();

    void on_comboBox_Mode_currentIndexChanged(int index);

    void on_comboBox_Scale_currentIndexChanged(int index);

    void on_checkBox_Shutter_toggled(bool checked);

    void on_actionimport_XML_settings_triggered();

    void on_pushButton_2_clicked();


private:
    Ui::MainWindow *ui;
    LabelVideo mLabelVideo;
    IVideoThread *mOpenCV_videoCapture;

    /* Да сам объект QTimer */
    QTimer *timer;

    int err;
    int p_w;
    int p_h;
    int t_w;
    int t_h;

    std::vector<unsigned char> palette_image;
    std::vector<unsigned short> thermal_data;

    int mTemperatureRangeFlag;

    //-------------------------------//

    // Setup SimpleBlobDetector parameters.
    cv::SimpleBlobDetector::Params params;
};
#endif // MAINWINDOW_H
