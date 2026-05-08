#include "startwindow.h"
#include "ui_startwindow.h"
#include "phototest.h"
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QTimer>
#include <QtMath>

StartWindow::StartWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::StartWindow)
{
    ui->setupUi(this);

    // 窗口基础
    this->setWindowTitle("✨ 颜值评估系统");
    this->setFixedSize(500, 380);

    //渐变背景
    this->setStyleSheet(R"(
        QWidget#StartWindow {
            background: qlineargradient(
                x1:0, y1:0, x2:1, y2:1,
                stop:0 #0a0a1a, stop:0.4 #1a1040, stop:1 #0d1b3e
            );
        }
    )");

    // 动态浮动光斑，用 QPainter 径向渐变
    auto makeGlowPixmap = [](const QColor &centerColor, int size) {
        QPixmap pix(size, size);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing);
        QPointF center(size / 2.0, size / 2.0);
       QRadialGradient g(center, size / 1.5);  // 中心亮斑
        g.setColorAt(0, centerColor);
        g.setColorAt(1, Qt::transparent);
        p.setBrush(g);
        p.setPen(Qt::NoPen);
        p.drawEllipse(pix.rect());
        p.end();
        return pix;
    };

    QLabel *glow1 = new QLabel(this);
    QLabel *glow2 = new QLabel(this);
    QLabel *glow3 = new QLabel(this);
    QLabel *glow4 = new QLabel(this);

    int glowSize = 200;
    glow1->setFixedSize(glowSize, glowSize);
    glow2->setFixedSize(glowSize, glowSize);
    glow3->setFixedSize(glowSize, glowSize);
    glow4->setFixedSize(glowSize, glowSize);

    // 建议值
    glow1->setPixmap(makeGlowPixmap(QColor(160, 120, 255, 150), glowSize));
    glow2->setPixmap(makeGlowPixmap(QColor(255, 100, 160, 140), glowSize));
    glow3->setPixmap(makeGlowPixmap(QColor(80, 180, 255, 130), glowSize));
    glow4->setPixmap(makeGlowPixmap(QColor(255, 180, 80, 130), glowSize));
    // 透明鼠标事件 + 置于底层
    for (auto *g : { glow1, glow2, glow3, glow4 }) {
        g->setAttribute(Qt::WA_TransparentForMouseEvents);
        g->setStyleSheet("background: transparent;");   // ← 关键
        g->setAttribute(Qt::WA_TranslucentBackground);  // ← 双保险
        g->lower();
    }

    glow1->move(10, 10);
    glow2->move(300, 100);
    glow3->move(100, 200);
    glow4->move(350, 30);

    // 动画：各光斑沿正弦轨迹移动
    QTimer *glowTimer = new QTimer(this);
    glowTimer->setInterval(50);
    float t = 0;
    connect(glowTimer, &QTimer::timeout, this, [=]() mutable {
        t += 0.02f;
        if (t > 2 * M_PI) t -= 2 * M_PI;
        // 动画里振幅
        glow1->move(10  + 40 * qSin(t * 1.3f),        10  + 35 * qCos(t * 1.7f));
        glow2->move(300 + 50 * qSin(t * 0.8f + 1),    100 + 45 * qCos(t * 1.2f + 2));
        glow3->move(100 + 40 * qSin(t * 1.1f + 3),    200 + 45 * qCos(t * 0.9f + 1));
        glow4->move(350 + 50 * qSin(t * 0.6f + 2),    30  + 40 * qCos(t * 1.5f + 0.5f));
    });
    glowTimer->start();

    // 标题区
    QLabel *titleLabel = new QLabel("✨ 颜值测评 ✨", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setGeometry(0, 30, 500, 60);
    QFont titleFont("Microsoft YaHei", 32, QFont::Bold);
    titleFont.setLetterSpacing(QFont::AbsoluteSpacing, 4);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #ffffff; background: transparent; border: none;");
    titleLabel->raise();
    auto *titleShadow = new QGraphicsDropShadowEffect(this);
    titleShadow->setBlurRadius(25);
    titleShadow->setOffset(0, 0);
    titleShadow->setColor(QColor(255, 255, 255, 100));
    titleLabel->setGraphicsEffect(titleShadow);

    // 呼吸动画定时器
    QTimer *breatheTimer = new QTimer(this);
    breatheTimer->setInterval(40);
    float breathePhase = 0;
    connect(breatheTimer, &QTimer::timeout, this, [=]() mutable {
        breathePhase += 0.05f;
        int alpha = 60 + 120 * qSin(breathePhase);
        titleShadow->setColor(QColor(255, 255, 255, alpha));
        titleShadow->setBlurRadius(30);              // 加模糊
    });

    breatheTimer->start();
    // 副标题
    QLabel *subtitle = new QLabel(" 68点面部分析", this);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setGeometry(0, 85, 500, 30);
    subtitle->setStyleSheet("color: #aaccff; font-size: 14px; background: transparent; letter-spacing: 2px;");

    // 胶囊按钮
    QString btnStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #ff2a6d, stop:1 #d300ff);
            border: 2px solid rgba(255,255,255,50%);
            color: white;
            font-size: 17px;
            font-weight: bold;
            border-radius: 25px;   /* 高度50px的一半 => 完美胶囊 */
            padding: 0px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #ff4085, stop:1 #e040ff);
            border-color: rgba(255,255,255,80%);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #c01b4d, stop:1 #a000c0);
        }
    )";

    // 照片测试按钮
    QPushButton *btnPhoto = ui->photoTestBtn;
    btnPhoto->setText("📷  照片测试");
    btnPhoto->setStyleSheet(btnStyle);
    btnPhoto->setCursor(Qt::PointingHandCursor);
    btnPhoto->setGeometry(80, 180, 140, 50);
    auto *shadow1 = new QGraphicsDropShadowEffect(this);
    shadow1->setBlurRadius(25);
    shadow1->setOffset(0, 4);
    shadow1->setColor(QColor(255, 42, 109, 120));
    btnPhoto->setGraphicsEffect(shadow1);

    // 摄像头测试按钮
    QPushButton *btnVideo = ui->videoTestBtn;
    btnVideo->setText("🎥  实时测试");
    btnVideo->setStyleSheet(btnStyle);
    btnVideo->setCursor(Qt::PointingHandCursor);
    btnVideo->setGeometry(280, 180, 140, 50);
    auto *shadow2 = new QGraphicsDropShadowEffect(this);
    shadow2->setBlurRadius(25);
    shadow2->setOffset(0, 4);
    shadow2->setColor(QColor(255, 42, 109, 120));
    btnVideo->setGraphicsEffect(shadow2);

    //悬停放大动画
    class HoverEnlarge : public QObject {
    public:
        HoverEnlarge(QPushButton *btn, QObject *parent) : QObject(parent), m_btn(btn) {
            m_origGeo = btn->geometry();
            btn->installEventFilter(this);
        }
    protected:
        bool eventFilter(QObject *obj, QEvent *e) override {
            if (obj == m_btn) {
                if (e->type() == QEvent::Enter) {
                    QRect larger = m_origGeo.adjusted(-5, -3, 5, 3);
                    auto *anim = new QPropertyAnimation(m_btn, "geometry");
                    anim->setDuration(200);
                    anim->setStartValue(m_btn->geometry());
                    anim->setEndValue(larger);
                    anim->start(QAbstractAnimation::DeleteWhenStopped);
                }
                else if (e->type() == QEvent::Leave) {
                    auto *anim = new QPropertyAnimation(m_btn, "geometry");
                    anim->setDuration(200);
                    anim->setStartValue(m_btn->geometry());
                    anim->setEndValue(m_origGeo);
                    anim->start(QAbstractAnimation::DeleteWhenStopped);
                }
            }
            return QObject::eventFilter(obj, e);
        }
    private:
        QPushButton *m_btn;
        QRect m_origGeo;
    };
    new HoverEnlarge(btnPhoto, this);
    new HoverEnlarge(btnVideo, this);

    //  装饰分隔线
    auto addLine = [this](int y) {
        QLabel *line = new QLabel(this);
        line->setGeometry(50, y, 400, 3);
        line->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
                            "stop:0 transparent, stop:0.5 rgba(255,255,255,60%), stop:1 transparent);"
                            "border: none;");
        line->setAttribute(Qt::WA_TransparentForMouseEvents);
    };
    addLine(280);

    // 底部提示
    QLabel *version = ui->label_2;
    version->setText("娱乐评价，仅供参考");
    version->setAlignment(Qt::AlignCenter);
    version->setGeometry(0, 300, 500, 25);
    version->setStyleSheet("color: #7a8aa0; font-size: 13px; background: transparent;");

    // 初始化
    photoTest = nullptr;
    videoTest = nullptr;
}
// 析构函数
StartWindow::~StartWindow()
{
    delete ui;
    if (photoTest) delete photoTest;
    if (videoTest) delete videoTest;
}

// 槽函数
void StartWindow::on_photoTestBtn_clicked()
{
    if (!photoTest) {
        photoTest = new PhotoTest(this);
    }
    photoTest->show();
    this->hide();
}

void StartWindow::on_videoTestBtn_clicked()
{
    if (!videoTest) {
        videoTest = new VideoTest(this);
    }
    videoTest->show();
    this->hide();
}