#include "phototest.h"
#include "startwindow.h"
#include "facialfeatureanalyzer.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QtMath>
#include <QEvent>

PhotoTest::PhotoTest(StartWindow *startWindow, QWidget *parent)
    : QMainWindow(parent)
    , m_startWindow(startWindow)
{
    setWindowTitle("✨ 颜值评估系统");
    setFixedSize(960, 750);

    // 全局背景样式
    applyGlobalStyle();

    // 搭建全部 UI（照片框、按钮、评分面板）
    setupUI();

}

void PhotoTest::applyGlobalStyle()
{
    setStyleSheet(R"(
        QMainWindow {
            background: qlineargradient(
                x1:0, y1:0, x2:1, y2:1,
                stop:0 #0a0a1a, stop:0.4 #1a1040, stop:1 #0d1b3e
            );
        }
    )");
}

void PhotoTest::setupUI()
{
    // 中央容器
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(30);

    // 左侧：照片框 + 按钮
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->setSpacing(20);

    // 照片框
    photoFrame = new QLabel;
    photoFrame->setFixedSize(500, 600);
    photoFrame->setAlignment(Qt::AlignCenter);
    photoFrame->setText("📷 点击下方按钮选择照片");
    photoFrame->setCursor(Qt::PointingHandCursor);

    // 高级粗边框样式
    photoFrame->setStyleSheet(R"(
    QLabel {
        background: rgba(18, 22, 35, 0.88);
        border: 3px solid #7a8b9f;
        border-radius: 28px;
        color:  #aaccff;
        font-size: 20px;
        font-weight: 600;
        letter-spacing: 1px;
        padding: 18px;
    }
    QLabel:hover {
        border-color: #b9a68c;
        background: rgba(25, 30, 45, 0.93);
        color: #ffffff;
    }
)");

    // 精致的静态阴影
    auto *frameShadow = new QGraphicsDropShadowEffect(photoFrame);
    frameShadow->setBlurRadius(28);
    frameShadow->setOffset(0, 8);
    frameShadow->setColor(QColor(0, 0, 0, 140));
    photoFrame->setGraphicsEffect(frameShadow);

    leftLayout->addWidget(photoFrame, 0, Qt::AlignCenter);




    // 按钮行
    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->setSpacing(30);

    QString neonBtnStyle = R"(
    QPushButton {
        background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                    stop:0 #ff2a6d, stop:1 #d300ff);
        border: 2px solid rgba(255,255,255,40%);
        color: white;
        font-size: 16px;
        font-weight: bold;
        border-radius: 25px;
        padding: 12px 30px;
        min-width: 180px;
        min-height: 50px;
    }
    QPushButton:hover {
        background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                    stop:0 #ff4085, stop:1 #e040ff);
        border-color: rgba(255,255,255,70%);
        font-size: 18px;
        padding: 14px 34px;      /* 略微扩大内边距，让文字居中时视觉更大 */
    }
    QPushButton:pressed {
        background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                    stop:0 #c01b4d, stop:1 #a000c0);
        font-size: 16px;         /* 按下时恢复，产生按压感 */
        padding: 12px 30px;
    }
    QPushButton:disabled {
        background: #555555;
        border-color: #777777;
        color: #aaaaaa;
    }
)";

    btnSelect = new QPushButton("📷 选择图片");
    btnSelect->setStyleSheet(neonBtnStyle);
    btnSelect->setCursor(Qt::PointingHandCursor);
    btnSelect->setFixedSize(180, 50);
    auto *s1 = new QGraphicsDropShadowEffect(this);
    s1->setBlurRadius(20);
    s1->setOffset(0, 4);
    s1->setColor(QColor(255, 42, 109, 100));
    btnSelect->setGraphicsEffect(s1);

    btnDetect = new QPushButton("🔍 开始检测");
    btnDetect->setStyleSheet(neonBtnStyle);
    btnDetect->setCursor(Qt::PointingHandCursor);
    btnDetect->setFixedSize(180, 50);
    btnDetect->setEnabled(false);
    auto *s2 = new QGraphicsDropShadowEffect(this);
    s2->setBlurRadius(20);
    s2->setOffset(0, 4);
    s2->setColor(QColor(255, 42, 109, 100));
    btnDetect->setGraphicsEffect(s2);

    btnRow->addWidget(btnSelect);
    btnRow->addWidget(btnDetect);
    leftLayout->addLayout(btnRow);

    mainLayout->addLayout(leftLayout);

    // 右侧：评分面板 + 返回按钮
    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->setSpacing(20);

    // 分数面板
    scorePanel = new ScoreDisplay;
    scorePanel->setFixedWidth(340);
    rightLayout->addWidget(scorePanel);

    rightLayout->addStretch();

    // 返回按钮
    btnBack = new QPushButton("↩ 返回主页");
    btnBack->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            border: 1px solid rgba(255,255,255,0.25);
            color: #cccccc;
            font-size: 14px;
            border-radius: 20px;
            padding: 8px 24px;
        }
        QPushButton:hover {
            background: rgba(255,255,255,0.08);
            color: white;
        }
    )");
    btnBack->setCursor(Qt::PointingHandCursor);
    btnBack->setFixedSize(160, 40);
    rightLayout->addWidget(btnBack, 0, Qt::AlignRight);

    mainLayout->addLayout(rightLayout);

    // 信号连接
    connect(btnSelect, &QPushButton::clicked, this, &PhotoTest::on_selectBtn_clicked);
    connect(btnDetect, &QPushButton::clicked, this, &PhotoTest::on_detectBtn_clicked);
    connect(btnBack,  &QPushButton::clicked, this, &PhotoTest::on_backBtn_clicked);


}

void PhotoTest::animatePhotoAppear()
{
    auto *fadeEffect = new QGraphicsOpacityEffect(this);
    photoFrame->setGraphicsEffect(fadeEffect);  // 暂时替换阴影
    fadeEffect->setOpacity(0.0);

    auto *anim = new QPropertyAnimation(fadeEffect, "opacity", this);
    anim->setDuration(600);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    // 动画结束后重新加上阴影
    connect(anim, &QPropertyAnimation::finished, this, [this]() {
        QGraphicsDropShadowEffect *s = new QGraphicsDropShadowEffect(this);
        s->setBlurRadius(30);
        s->setOffset(0, 6);
        s->setColor(QColor(0,0,0,150));
        photoFrame->setGraphicsEffect(s);
    });
}
// 槽函数
void PhotoTest::on_selectBtn_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "选择图片", "",
                                                "图片 (*.png *.jpg *.jpeg)");
    if (path.isEmpty()) return;

    currentImagePath = path;
    QPixmap pix(path);
    photoFrame->setPixmap(pix.scaled(photoFrame->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    photoFrame->setText("");

    btnSelect->setText("↻ 重新选择");
    btnDetect->setEnabled(true);
    animatePhotoAppear();
}

void PhotoTest::on_detectBtn_clicked()
{
    doDetectionAndScoring();
}

void PhotoTest::doDetectionAndScoring()
{
    if (currentImagePath.isEmpty()) return;

    // 读取图片 (BGR)
    cv::Mat bgrImage = cv::imread(currentImagePath.toLocal8Bit().toStdString());
    if (bgrImage.empty()) {
        QMessageBox::warning(this, "错误", "无法读取图片！");
        return;
    }

    // 检测 + 对齐
    if (!detector.detect(bgrImage)) {
        QMessageBox::warning(this, "提示", "未检测到合适的人脸！");
        scorePanel->clear();
        return;
    }

    //  创建带关键点的显示图像
    // 获取关键点（原始坐标）
    auto rawPoints = detector.getLandmarks();

    // 在原图上画点，半径按图像宽度动态计算（例如占宽度的 0.4%）
    int imgWidth = bgrImage.cols;
    int radius = std::max(2, cvRound(imgWidth * 0.004));  // 至少2像素，防止小图看不见

    cv::Mat displayImage = bgrImage.clone();
    FaceDetector::drawLandmarks(displayImage, rawPoints, cv::Scalar(0, 255, 0), radius);

    // 转为 QImage 并显示在 photoFrame 上
    cv::cvtColor(displayImage, displayImage, cv::COLOR_BGR2RGB);
    QImage qimg(displayImage.data, displayImage.cols, displayImage.rows,
                displayImage.step, QImage::Format_RGB888);
    photoFrame->setPixmap(QPixmap::fromImage(qimg).scaled(photoFrame->size(),
                                                          Qt::KeepAspectRatio,
                                                          Qt::SmoothTransformation));

    // 多脸提示
    if (detector.getFaceCount() > 1) {
        QMessageBox::information(this, "提示", "检测到多张人脸，已自动选取最大人脸评分");
    }

    // 获取对齐后的关键点与正脸，用于评分
    auto alignedPts = detector.getAlignedLandmarks();
    auto alignedFace = detector.getAlignedFace();
    // 理论上 detect() 成功这里数据一定有效，但仍做一次保护
    if (alignedPts.size() < 68 || alignedFace.empty()) {
        QMessageBox::warning(this, "错误", "人脸数据异常，无法评分");
        return;
    }

    // 计算总分
    double total = m_beauty.calculateTotalScore(alignedPts, alignedFace);
    scorePanel->updateScore(
        m_beauty.sanTingScore(),
        m_beauty.wuYanScore(),
        m_beauty.symmetryScore(),
        m_beauty.skinScore(),
        m_beauty.featureProportionsScore(),
        total
        );

    //五官分析
    FacialFeatures feat = m_analyzer.analyze(alignedPts);
    scorePanel->setFacialFeatures(feat);
}

void PhotoTest::on_backBtn_clicked()
{
    if (m_startWindow) {
        m_startWindow->show();
        this->hide();
    }
    // 重置
    currentImagePath.clear();
    photoFrame->clear();
    photoFrame->setText("📷 点击下方按钮选择照片");
    scorePanel->clear();
    btnSelect->setText("📷 选择图片");
    btnDetect->setEnabled(false);
}