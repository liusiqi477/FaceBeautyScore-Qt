#include "videotest.h"
#include "ui_videotest.h"
#include <QMessageBox>
#include <QDebug>
#include <opencv2/opencv.hpp>

VideoTest::VideoTest(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::VideoTest)
{
    ui->setupUi(this);
    this->setWindowTitle("实时视频颜值测评");
    scoreDisplay = new ScoreDisplay(ui->widget_score);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &VideoTest::updateFrame);
}

VideoTest::~VideoTest()
{
    stopCamera();
    delete ui;
}

void VideoTest::on_openCameraBtn_clicked()
{
    if (cap.isOpened()) {
        stopCamera();
        ui->openCameraBtn->setText("打开摄像头");
        return;
    }

    // ✅ 【修复1】摄像头用 CAP_DSHOW 模式，Windows 稳定打开
    cap.open(0, cv::CAP_DSHOW);

    if (!cap.isOpened()) {
        QMessageBox::warning(this, "错误", "无法打开摄像头！");
        return;
    }

    timer->start(40); // ✅ 【修复2】降低帧率，不卡不崩
    ui->openCameraBtn->setText("关闭摄像头");
}

void VideoTest::updateFrame()
{
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) return;

    // ✅ 【修复3】必须克隆一份，防止原图被改写
    cv::Mat process = frame.clone();

    try {
        // 检测68点
        detector.detect68Points_Mat(process);

        // 画点
        auto points = detector.getLandmarks();
        for (auto& p : points) {
            cv::circle(frame, p, 2, cv::Scalar(0, 255, 0), -1);
        }

        // 评分
        auto alignedPts = detector.getAlignedLandmarks();
        auto alignedFace = detector.getAlignedFace();
        double total = 0;

        if (alignedPts.size() == 68 && !alignedFace.empty()) {
            total = scorer.calculateTotalScore(alignedPts, alignedFace);
            scoreDisplay->updateScore(
                scorer.sanTingScore(),
                scorer.wuYanScore(),
                scorer.symmetryScore(),
                scorer.skinScore(),
                scorer.featureProportionsScore(),
                total
                );

            // ===== 五官分析（新增） =====
            FacialFeatureAnalyzer analyzer;
            FacialFeatures feat = analyzer.analyze(alignedPts);
            scoreDisplay->setFacialFeatures(feat);

        } else {
            scoreDisplay->clear();
        }
    } catch (...) {
        // ✅ 【修复4】崩溃拦截，不会退出
        scoreDisplay->clear();
    }

    // 显示画面
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    QImage qimg(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    ui->label->setPixmap(QPixmap::fromImage(qimg).scaled(ui->label->size(), Qt::KeepAspectRatio));
}

void VideoTest::stopCamera()
{
    timer->stop();
    if (cap.isOpened()) cap.release();
    // scoreDisplay->clear();
}

void VideoTest::on_backBtn_clicked()
{
    stopCamera();
    this->hide();
    parentWidget()->show();
}