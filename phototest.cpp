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

    // 添加动态光斑
    addGlowEffects();

    // 初始化检测器和评分器
    //（假设 detector、m_beauty、m_analyzer 已在头文件中声明）
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

    // ========== 左侧：照片框 + 按钮 ==========
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->setSpacing(20);

    // 照片框
    photoFrame = new QLabel;
    photoFrame->setFixedSize(500, 600);
    photoFrame->setAlignment(Qt::AlignCenter);
    photoFrame->setText("📷 点击下方按钮选择照片");
    photoFrame->setCursor(Qt::PointingHandCursor);

    // ---------- 高级粗边框样式 ----------
    photoFrame->setStyleSheet(R"(
    QLabel {
        background: rgba(18, 22, 35, 0.88);        /* 深蓝黑玻璃底 */
        border: 3px solid #7a8b9f;                 /* 高级灰蓝粗边框 */
        border-radius: 28px;
        color:  #aaccff;
        font-size: 20px;
        font-weight: 600;
        letter-spacing: 1px;
        padding: 18px;
    }
    QLabel:hover {
        border-color: #b9a68c;                     /* 鼠标悬停暖金边 */
        background: rgba(25, 30, 45, 0.93);
        color: #ffffff;
    }
)");

    // ---------- 精致的静态阴影（无动画） ----------
    auto *frameShadow = new QGraphicsDropShadowEffect(photoFrame);
    frameShadow->setBlurRadius(28);
    frameShadow->setOffset(0, 8);
    frameShadow->setColor(QColor(0, 0, 0, 140));       /* 深色投影增加立体感 */
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
    btnSelect->setFixedSize(100, 50);
    auto *s1 = new QGraphicsDropShadowEffect(this);
    s1->setBlurRadius(20);
    s1->setOffset(0, 4);
    s1->setColor(QColor(255, 42, 109, 100));
    btnSelect->setGraphicsEffect(s1);

    btnDetect = new QPushButton("🔍 开始检测");
    btnDetect->setStyleSheet(neonBtnStyle);
    btnDetect->setCursor(Qt::PointingHandCursor);
    btnDetect->setFixedSize(100, 50);
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

    // ========== 右侧：评分面板 + 返回按钮 ==========
    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->setSpacing(20);

    // 分数面板（自定义的 ScoreDisplay 卡片）
    scorePanel = new ScoreDisplay;
    scorePanel->setFixedWidth(340);  // 宽度配合卡片样式
    rightLayout->addWidget(scorePanel);

    rightLayout->addStretch();

    // 返回按钮（透明边框风格）
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
// 动态光斑：四个半透明圆点在背景中缓慢飘移
void PhotoTest::addGlowEffects()
{
    auto createGlow = [&](int x, int y, QString color) {
        QLabel *glow = new QLabel(this);
        glow->setFixedSize(200, 200);
        glow->setStyleSheet(QString("background: radialgradient(cx:0.5, cy:0.5, radius:0.5, "
                                    "stop:0 %1, stop:1 transparent); border: none;").arg(color));
        glow->setAttribute(Qt::WA_TransparentForMouseEvents);
        glow->move(x, y);
        glow->lower();  // 放在最底层
        return glow;
    };

    QLabel *g1 = createGlow(60, 50,  "rgba(160,120,255,25%)");
    QLabel *g2 = createGlow(700, 100, "rgba(255,100,160,20%)");
    QLabel *g3 = createGlow(200, 450, "rgba(80,180,255,18%)");
    QLabel *g4 = createGlow(650, 480, "rgba(255,180,80,15%)");

    glowTimer = new QTimer(this);
    glowTimer->setInterval(50);
    float t = 0;
    connect(glowTimer, &QTimer::timeout, this, [=]() mutable {
        t += 0.02f;
        if (t > 2 * M_PI) t -= 2 * M_PI;
        g1->move(60  + 20 * qSin(t * 1.3f),  50  + 15 * qCos(t * 1.7f));
        g2->move(700 + 25 * qSin(t * 0.8f + 1), 100 + 20 * qCos(t * 1.2f + 2));
        g3->move(200 + 18 * qSin(t * 1.1f + 3), 450 + 22 * qCos(t * 0.9f + 1));
        g4->move(650 + 22 * qSin(t * 0.6f + 2), 480 + 18 * qCos(t * 1.5f + 0.5f));
    });
    glowTimer->start();
}
void PhotoTest::animatePhotoAppear()
{
    auto *fadeEffect = new QGraphicsOpacityEffect(this);
    photoFrame->setGraphicsEffect(fadeEffect);  // 暂时替换阴影（动画结束后换回）
    fadeEffect->setOpacity(0.0);

    auto *anim = new QPropertyAnimation(fadeEffect, "opacity", this);
    anim->setDuration(600);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    // 动画结束后重新加上阴影（连接信号）
    connect(anim, &QPropertyAnimation::finished, this, [this]() {
        // 重新设置一个阴影效果（如果之前被你替换了的话）
        QGraphicsDropShadowEffect *s = new QGraphicsDropShadowEffect(this);
        s->setBlurRadius(30);
        s->setOffset(0, 6);
        s->setColor(QColor(0,0,0,150));
        photoFrame->setGraphicsEffect(s);
    });
}
// ====== 业务槽函数 ======
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

    QImage result = detector.detect68Points(currentImagePath);
    if (result.isNull()) {
        QMessageBox::warning(this, "提示", "未检测到人脸！");
        scorePanel->clear();
        return;
    }

    // 显示带68点的原图
    photoFrame->setPixmap(QPixmap::fromImage(result).scaled(photoFrame->size(), Qt::KeepAspectRatio));

    if (detector.getFaceCount() > 1) {
        QMessageBox::information(this, "提示", "检测到多张人脸，已自动选取最大人脸评分");
    }

    auto alignedPts = detector.getAlignedLandmarks();
    auto alignedFace = detector.getAlignedFace();
    if (alignedPts.size() < 68 || alignedFace.empty()) {
        QMessageBox::warning(this, "错误", "人脸数据异常，无法评分");
        return;
    }

    double total = m_beauty.calculateTotalScore(alignedPts, alignedFace);
    scorePanel->updateScore(
        m_beauty.sanTingScore(),
        m_beauty.wuYanScore(),
        m_beauty.symmetryScore(),
        m_beauty.skinScore(),
        m_beauty.featureProportionsScore(),
        total
        );

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