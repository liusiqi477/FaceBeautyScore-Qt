#ifndef VIDEOTEST_H
#define VIDEOTEST_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <opencv2/opencv.hpp>
#include "facedetector.h"
#include "beautyscorer.h"
#include "scoredisplay.h"
#include "facialfeatureanalyzer.h"
#include <QPushButton>

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
    void on_openCameraBtn_clicked();
    void updateFrame();
    void on_backBtn_clicked();

private:
    void setupUI();
    void applyGlobalStyle();
    // void addGlowEffects();

    // 新控件
    QLabel *videoFrame;      // 原 ui->label
    QPushButton *btnCamera;  // 原 ui->openCameraBtn
    QPushButton *btnBack;    // 原 ui->backBtn
    QLabel *hintLabel;       // 原 ui->hintLabel

private:
    void resetEvaluation();
    void stopCamera();
    QString generateHint();       // 根据锁定分数生成提示语

    Ui::VideoTest *ui;

    cv::VideoCapture cap;
    QTimer* timer;
    FaceDetector detector;
    BeautyScorer scorer;
    ScoreDisplay* scoreDisplay;
    FacialFeatureAnalyzer featureAnalyzer;

    //  状态
    enum class EvalState { Evaluating, Locked };
    EvalState evalState = EvalState::Evaluating;

    // 单项锁定标志 & 稳定计数
    bool lockSanTing    = false;
    bool lockWuYan      = false;
    bool lockSymmetry   = false;
    bool lockSkin       = false;
    bool lockProportions = false;

    int stableSanTing    = 0;
    int stableWuYan      = 0;
    int stableSymmetry   = 0;
    int stableSkin       = 0;
    int stableProportions = 0;

    double lastSanTing    = 0.0;
    double lastWuYan      = 0.0;
    double lastSymmetry   = 0.0;
    double lastSkin       = 0.0;
    double lastProportions = 0.0;

    // 锁定后的最终值
    double lockedSanTing    = 0.0;
    double lockedWuYan      = 0.0;
    double lockedSymmetry   = 0.0;
    double lockedSkin       = 0.0;
    double lockedProportions = 0.0;

    // EMA 平滑当前值
    double smoothSanTing     = 0.0;
    double smoothWuYan       = 0.0;
    double smoothSymmetry    = 0.0;
    double smoothSkin        = 0.0;
    double smoothProportions = 0.0;

    static constexpr double alpha = 0.2;            // 平滑系数
    static constexpr int requiredStableFrames = 40; // 单项稳定帧数
    static constexpr double itemThreshold = 1.5;    // 单项波动阈值（分）
};

#endif // VIDEOTEST_H