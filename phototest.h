#ifndef PHOTOTEST_H
#define PHOTOTEST_H

#include <QMainWindow>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include "facedetector.h"
#include "beautyscorer.h"
#include "scoredisplay.h"
#include "facialfeatureanalyzer.h"
//前向声明 StartWindow，避免循环依赖
class StartWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class PhotoTest; }
QT_END_NAMESPACE


enum class PhotoState {
    Idle,          // 未选择图片
    ImageLoaded,   // 已选图，未检测
    Detected       // 已完成68点检测
};
class PhotoTest : public QMainWindow
{
    Q_OBJECT
public:
    explicit PhotoTest(StartWindow *startWindow, QWidget *parent = nullptr);
    ~PhotoTest(){};

private slots:
    void on_backBtn_clicked();
    void on_selectBtn_clicked();
    void on_detectBtn_clicked();

private:
    void setupUI();
     void applyGlobalStyle();
    void doDetectionAndScoring();
    void animatePhotoAppear();
    // UI 控件指针
    QLabel *photoFrame;
    QPushButton *btnSelect, *btnDetect, *btnBack;
    ScoreDisplay *scorePanel;
    QTimer *glowTimer;
    QPropertyAnimation *m_borderGlowAnim;
    // 业务
    StartWindow *m_startWindow;
    FaceDetector detector;
    BeautyScorer m_beauty;
    FacialFeatureAnalyzer m_analyzer;
    QString currentImagePath;
};

#endif // PHOTOTEST_H

