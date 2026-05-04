#ifndef PHOTOTEST_H
#define PHOTOTEST_H

#include <QMainWindow>
#include "facedetector.h"
#include "beautyscorer.h"
#include "scoredisplay.h"
#include "facialfeatureanalyzer.h"
// 【关键】前向声明 StartWindow，避免循环依赖
class StartWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class PhotoTest; }
QT_END_NAMESPACE

class PhotoTest : public QMainWindow
{
    Q_OBJECT

public:
    // 【修正】指针语法 + 显式类型
    explicit PhotoTest(StartWindow *startWindow, QWidget *parent = nullptr);
    ~PhotoTest();

private slots:
    void on_selectBtn_clicked();
    void on_detectBtn_clicked();
    void on_backBtn_clicked();

private:
    Ui::PhotoTest *ui;
    FaceDetector detector;
    QString currentImagePath;
    StartWindow *m_startWindow;  // 指针成员
    BeautyScorer m_beauty;  // 评分对象
    ScoreDisplay *m_scoreDisplay;  // 分数显示
    FacialFeatureAnalyzer featureAnalyzer;   // 新增成员
};

#endif // PHOTOTEST_H