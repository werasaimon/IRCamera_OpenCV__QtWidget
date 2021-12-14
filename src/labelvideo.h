#ifndef LABELVIDEO_H
#define LABELVIDEO_H

#include <QLabel>
#include <QObject>


class IVideoThread;
class LabelVideo : public QLabel
{
    Q_OBJECT
public:
    LabelVideo(QWidget *parent = nullptr);

    void setOpenCV_videoCapture(IVideoThread *newOpenCV_videoCapture);

private:

    IVideoThread *mOpenCV_videoCapture;
};

#endif // LABELVIDEO_H
