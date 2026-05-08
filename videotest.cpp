#include "videotest.h"
#include "ui_videotest.h"
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QtMath>

VideoTest::VideoTest(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::VideoTest)
{
    ui->setupUi(this);
    setWindowTitle("✨ 实时颜值测评");
    setFixedSize(1100, 650);

    // 全局背景样式
    setStyleSheet(R"(
        QMainWindow {
            background: qlineargradient(
                x1:0, y1:0, x2:1, y2:1,
                stop:0 #0b1320, stop:0.5 #1a2a4a, stop:1 #0d1f3c
            );
        }
    )");

    // 重新布局整个界面
    setupUI();


    //  创建定时器和评分面板
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &VideoTest::updateFrame);

    resetEvaluation();
}

VideoTest::~VideoTest()
{
    stopCamera();
    delete ui;
}

// 界面搭建
void VideoTest::setupUI()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(25, 25, 25, 25);
    mainLayout->setSpacing(25);

    // 左侧：视频框 + 按钮 + 提示
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->setSpacing(15);

    // 视频框 (代替原来的 ui->label)
    videoFrame = new QLabel;
    videoFrame->setFixedSize(580, 500);
    videoFrame->setAlignment(Qt::AlignCenter);
    videoFrame->setStyleSheet(R"(
        QLabel {
            background: rgba(0, 0, 0, 0.35);
            border: 2px solid rgba(255,255,255,0.15);
            border-radius: 24px;
            color: #aaccff;
            font-size: 16px;
        }
    )");
    auto *frameShadow = new QGraphicsDropShadowEffect(this);
    frameShadow->setBlurRadius(25);
    frameShadow->setOffset(0, 4);
    frameShadow->setColor(QColor(0, 0, 0, 160));
    videoFrame->setGraphicsEffect(frameShadow);
    leftLayout->addWidget(videoFrame, 0, Qt::AlignCenter);

    // 按钮行
    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->setSpacing(100);

    QString neonStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #ff2a6d, stop:1 #d300ff);
            border: 2px solid rgba(255,255,255,40%);
            color: white;
            font-size: 16px;
            font-weight: bold;
            border-radius: 25px;
            padding: 12px 30px;
            min-width: 160px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #ff4085, stop:1 #e040ff);
            border-color: rgba(255,255,255,70%);
            font-size: 18px;
            padding: 14px 34px;
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #c01b4d, stop:1 #a000c0);
            font-size: 16px;
            padding: 12px 30px;
        }
    )";

    btnCamera = new QPushButton("🎥 打开摄像头");
    btnCamera->setStyleSheet(neonStyle);
    btnCamera->setCursor(Qt::PointingHandCursor);
    btnCamera->setFixedSize(180, 50);
    auto *s1 = new QGraphicsDropShadowEffect(this);
    s1->setBlurRadius(20);
    s1->setOffset(0, 4);
    s1->setColor(QColor(255, 42, 109, 100));
    btnCamera->setGraphicsEffect(s1);

    btnBack = new QPushButton("↩ 返回主页");
    btnBack->setStyleSheet(neonStyle);
    btnBack->setCursor(Qt::PointingHandCursor);
    btnBack->setFixedSize(180, 50);

    btnBack->setGraphicsEffect(s1);

    //按钮位置
    btnRow->addStretch();
    btnRow->addWidget(btnCamera);
    btnRow->addWidget(btnBack);
    btnRow->addStretch();
    leftLayout->addLayout(btnRow);

    // 提示标签
    hintLabel = new QLabel;
    hintLabel->setStyleSheet("color: #ffb86c; font-size: 13px; background: transparent;");
    hintLabel->setWordWrap(true);
    hintLabel->setFixedWidth(580);
    hintLabel->hide();
    leftLayout->addWidget(hintLabel);

    mainLayout->addLayout(leftLayout);

    // 右侧：评分面板
    QVBoxLayout *rightLayout = new QVBoxLayout;
    scoreDisplay = new ScoreDisplay;
    scoreDisplay->setFixedWidth(360);
    rightLayout->addWidget(scoreDisplay);
    rightLayout->addStretch();
    mainLayout->addLayout(rightLayout);

    // 信号连接
    connect(btnCamera, &QPushButton::clicked, this, &VideoTest::on_openCameraBtn_clicked);
    connect(btnBack,   &QPushButton::clicked, this, &VideoTest::on_backBtn_clicked);
}



void VideoTest::on_openCameraBtn_clicked()
{
    if (cap.isOpened()) {
        stopCamera();
        resetEvaluation();
    }

    cap.open(0, cv::CAP_DSHOW);
    if (!cap.isOpened()) {
        QMessageBox::warning(this, "错误", "无法打开摄像头！");
        return;
    }

    timer->start(25);
    btnCamera->setText("⏹ 停止测评");
}

void VideoTest::updateFrame()
{
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) return;

    cv::Mat process = frame.clone();

    try {
        // 使用 detect 接口
        bool detected = detector.detect(process);

        // 画68点
        auto points = detector.getLandmarks();
        for (auto& p : points)
            cv::circle(frame, p, 2, cv::Scalar(0, 255, 0), -1);

        auto alignedPts = detector.getAlignedLandmarks();
        auto alignedFace = detector.getAlignedFace();

        // 只有在检测成功且对齐数据有效时才进行评分
        if (detected && evalState == EvalState::Evaluating && alignedPts.size() == 68 && !alignedFace.empty()) {
            double total = scorer.calculateTotalScore(alignedPts, alignedFace,
                                                      detector.getLandmarks(), frame);
            double sanTing     = scorer.sanTingScore();
            double wuYan       = scorer.wuYanScore();
            double symmetry    = scorer.symmetryScore();
            double skin        = scorer.skinScore();
            double proportions = scorer.featureProportionsScore();

            // EMA 平滑
            smoothSanTing     = alpha * sanTing     + (1.0 - alpha) * smoothSanTing;
            smoothWuYan       = alpha * wuYan       + (1.0 - alpha) * smoothWuYan;
            smoothSymmetry    = alpha * symmetry    + (1.0 - alpha) * smoothSymmetry;
            smoothSkin        = alpha * skin        + (1.0 - alpha) * smoothSkin;
            smoothProportions = alpha * proportions + (1.0 - alpha) * smoothProportions;

            // 锁定检测
            auto tryLock = [&](bool &lockFlag, int idx, double curVal,
                               double &lastCand, int &stableCount) {
                if (lockFlag) return;
                if (std::abs(curVal - lastCand) < itemThreshold) {
                    stableCount++;
                } else {
                    stableCount = 0;
                    lastCand = curVal;
                }
                if (stableCount >= requiredStableFrames) {
                    lockFlag = true;
                    switch (idx) {
                    case 0: lockedSanTing = curVal; break;
                    case 1: lockedWuYan = curVal; break;
                    case 2: lockedSymmetry = curVal; break;
                    case 3: lockedSkin = curVal; break;
                    case 4: lockedProportions = curVal; break;
                    }
                    scoreDisplay->setItemLocked(idx, true);
                }
            };

            tryLock(lockSanTing, 0, smoothSanTing, lastSanTing, stableSanTing);
            tryLock(lockWuYan, 1, smoothWuYan, lastWuYan, stableWuYan);
            tryLock(lockSymmetry, 2, smoothSymmetry, lastSymmetry, stableSymmetry);
            tryLock(lockSkin, 3, smoothSkin, lastSkin, stableSkin);
            tryLock(lockProportions, 4, smoothProportions, lastProportions, stableProportions);

            bool allLocked = lockSanTing && lockWuYan && lockSymmetry && lockSkin && lockProportions;
            if (allLocked) {
                evalState = EvalState::Locked;
                double finalTotal = lockedSanTing + lockedWuYan + lockedSymmetry +
                                    lockedSkin + lockedProportions;

                scoreDisplay->updateScore(lockedSanTing, lockedWuYan, lockedSymmetry,
                                          lockedSkin, lockedProportions, finalTotal);

                FacialFeatures feat = featureAnalyzer.analyze(alignedPts);
                scoreDisplay->setFacialFeatures(feat);

                QString hint = generateHint();
                if (hint.isEmpty()) {
                    hintLabel->hide();
                } else {
                    hintLabel->setText(hint);
                    hintLabel->show();
                }

                btnCamera->setText("🔄 重新测评");
            } else {
                scoreDisplay->updateScore(smoothSanTing, smoothWuYan, smoothSymmetry,
                                          smoothSkin, smoothProportions, -1);
                hintLabel->hide();
            }
        } else if (evalState == EvalState::Evaluating) {
            // 没有检测到人脸或数据无效则重置未锁定项的稳定计数
            if (!lockSanTing)    { stableSanTing = 0; lastSanTing = 0.0; }
            if (!lockWuYan)      { stableWuYan = 0; lastWuYan = 0.0; }
            if (!lockSymmetry)   { stableSymmetry = 0; lastSymmetry = 0.0; }
            if (!lockSkin)       { stableSkin = 0; lastSkin = 0.0; }
            if (!lockProportions){ stableProportions = 0; lastProportions = 0.0; }
        }
    } catch (...) {
        // 异常安全：重置未锁定项
        if (evalState == EvalState::Evaluating) {
            if (!lockSanTing)    stableSanTing = 0;
            if (!lockWuYan)      stableWuYan = 0;
            if (!lockSymmetry)   stableSymmetry = 0;
            if (!lockSkin)       stableSkin = 0;
            if (!lockProportions)stableProportions = 0;
        }
    }

    // 镜像显示
    cv::flip(frame, frame, 1);
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    QImage qimg(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    videoFrame->setPixmap(QPixmap::fromImage(qimg).scaled(videoFrame->size(),
                                                          Qt::KeepAspectRatio,
                                                          Qt::SmoothTransformation));
}

QString VideoTest::generateHint()
{
    QStringList tips;
    if (lockedSanTing     < 5.0) tips << "请拨开刘海露出额头，保持正面平视";
    if (lockedWuYan       < 5.0) tips << "请摘掉眼镜，避免遮挡眼周";
    if (lockedSymmetry    < 5.0) tips << "请保持正脸，避免侧转或歪头";
    if (lockedSkin        < 5.0) tips << "请确保光线均匀明亮，避免逆光";
    if (lockedProportions < 5.0) tips << "请保持自然表情，正视镜头";
    if (tips.isEmpty()) return "";
    return "💡 测评建议：" + tips.join("；");
}

void VideoTest::stopCamera()
{
    timer->stop();
    if (cap.isOpened()) cap.release();
}

void VideoTest::on_backBtn_clicked()
{
    stopCamera();
    hide();
    parentWidget()->show();
}

void VideoTest::resetEvaluation()
{
    evalState = EvalState::Evaluating;
    lockSanTing = lockWuYan = lockSymmetry = lockSkin = lockProportions = false;
    stableSanTing = stableWuYan = stableSymmetry = stableSkin = stableProportions = 0;
    lastSanTing = lastWuYan = lastSymmetry = lastSkin = lastProportions = 0.0;
    smoothSanTing = smoothWuYan = smoothSymmetry = smoothSkin = smoothProportions = 0.0;
    lockedSanTing = lockedWuYan = lockedSymmetry = lockedSkin = lockedProportions = 0.0;

    for (int i = 0; i < 5; ++i)
        scoreDisplay->setItemLocked(i, false);
    scoreDisplay->resetLockedFlags();
    scoreDisplay->clear();

    hintLabel->hide();
    btnCamera->setText("🎥 打开摄像头");
}
