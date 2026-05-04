#ifndef VIDEOTEST_H
#define VIDEOTEST_H

#include <QMainWindow>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include "facedetector.h"
#include "beautyscorer.h"
#include "scoredisplay.h"
#include "facialfeatureanalyzer.h"

namespace Ui {
class VideoTest;
}

class VideoTest : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideoTest(QWidget *parent = nullptr);
    ~VideoTest();

private slots:
    // 打开摄像头
    void on_openCameraBtn_clicked();
    // 定时器：读取每一帧
    void updateFrame();
    // 返回
    void on_backBtn_clicked();

private:
    Ui::VideoTest *ui;

    // opencv 摄像头
    cv::VideoCapture cap;
    QTimer* timer;

    // 你已经有的核心算法
    FaceDetector detector;
    BeautyScorer scorer;
    ScoreDisplay* scoreDisplay;
    FacialFeatureAnalyzer featureAnalyzer;   // 新增成员
    // 停止摄像头
    void stopCamera();
};

#endif // VIDEOTEST_H