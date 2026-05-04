#include "scoredisplay.h"
#include "facialfeatureanalyzer.h"   // 包含 FacialFeatures 完整定义
#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QLinearGradient>

ScoreDisplay::ScoreDisplay(QWidget *parent) : QWidget(parent)
{
    // 整体容器样式
    setStyleSheet(R"(
        ScoreDisplay {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                        stop:0 #2b2b2b, stop:1 #1e1e1e);
            border-radius: 20px;
        }
    )");
    setFixedWidth(320);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 25, 20, 20);

    // ========== 总分区域 ==========
    lab_total = new QLabel("--");
    lab_total->setAlignment(Qt::AlignCenter);
    lab_total->setStyleSheet(R"(
        QLabel {
            font-size: 48px;
            font-weight: bold;
            color: #ffffff;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #ff7eb3, stop:1 #ff758c);
            border-radius: 15px;
            padding: 10px;
        }
    )");
    auto *shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(15);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 150));
    lab_total->setGraphicsEffect(shadow);

    // 评语
    lab_comment = new QLabel("评语：--");
    lab_comment->setAlignment(Qt::AlignCenter);
    lab_comment->setStyleSheet("font-size: 16px; font-weight: bold; padding: 4px; color:#dddddd;");

    mainLayout->addWidget(lab_total);
    mainLayout->addWidget(lab_comment);
    mainLayout->addSpacing(10);

    // ========== 维度项 ==========
    auto createItem = [&](const QString &icon, const QString &name,
                          QLabel *&label, QProgressBar *&bar) {
        auto *hLayout = new QHBoxLayout;
        label = new QLabel(icon + " " + name + "  0/20");
        label->setStyleSheet("font-size: 15px; font-weight: bold; color: #dddddd;");
        hLayout->addWidget(label);

        bar = new QProgressBar;
        bar->setRange(0, 20);
        bar->setTextVisible(false);
        bar->setFixedHeight(16);
        bar->setStyleSheet(R"(
            QProgressBar {
                border: none;
                border-radius: 8px;
                background: #3a3a3a;
            }
            QProgressBar::chunk {
                border-radius: 8px;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                            stop:0 #56ab2f, stop:1 #a8e063);
            }
        )");
        hLayout->addWidget(bar);
        mainLayout->addLayout(hLayout);
    };

    createItem("🔺", "三庭", lab_san, bar_san);
    createItem("👁️", "五眼", lab_wu, bar_wu);
    createItem("🔄", "对称", lab_sym, bar_sym);
    createItem("✨", "皮肤", lab_skin, bar_skin);
    createItem("🎭", "协调", lab_feat, bar_feat);

    // 间距
    mainLayout->addSpacing(20);

    // 创建并添加五官分析卡片
    setupFeatureCard();
    mainLayout->addWidget(featureCard);

    setLayout(mainLayout);
    clear();
}

void ScoreDisplay::setupFeatureCard()
{
    featureCard = new QFrame;
    featureCard->setStyleSheet(R"(
        QFrame {
            background: rgba(25, 25, 45, 70%);
            border: 1px solid rgba(255,255,255,0.1);
            border-radius: 16px;
        }
    )");
    auto *cardShadow = new QGraphicsDropShadowEffect(this);
    cardShadow->setBlurRadius(20);
    cardShadow->setOffset(0, 4);
    cardShadow->setColor(QColor(0, 0, 0, 80));
    featureCard->setGraphicsEffect(cardShadow);

    auto *cardLayout = new QVBoxLayout(featureCard);
    cardLayout->setSpacing(10);
    cardLayout->setContentsMargins(16, 16, 16, 16);

    // 标题
    auto *title = new QLabel("🧩 五官特征");
    title->setStyleSheet("font-size:15px; font-weight:bold; color:#c3bef7; background:transparent;");
    cardLayout->addWidget(title);

    // 通用标签样式
    QString lblStyle = "font-size:13px; color:#d0d0e0; background:rgba(255,255,255,0.06); "
                       "border-radius:8px; padding:4px 10px;";

    // 创建一行两个标签的辅助函数
    auto addRow = [&](QLabel *&left, QLabel *&right,
                      const QString &leftText, const QString &rightText) {
        auto *row = new QHBoxLayout;
        row->setSpacing(12);          // 两个指标之间的间距
        left  = new QLabel(leftText);
        right = new QLabel(rightText);
        left->setStyleSheet(lblStyle);
        right->setStyleSheet(lblStyle);
        row->addWidget(left);
        row->addWidget(right);
        row->addStretch();           // 把剩余空间推到右边，让两个标签靠左紧凑排列
        cardLayout->addLayout(row);
    };

    // 8个指标，每行两个
    addRow(labEyeShape,  labEyeDist,  "眼型: --", "眼距: --");
    addRow(labEyeSize,   labNoseWing, "眼大小: --", "鼻翼: --");
    addRow(labLipThick,  labLipWidth, "唇厚: --", "唇宽: --");
    addRow(labBrowThick, labBrowDist, "眉粗: --", "眉眼距: --");
}

void ScoreDisplay::setFacialFeatures(const FacialFeatures &f)
{
    labEyeShape ->setText("眼型: " + formatFeature(f.eyeShape, "eyeShape"));
    labEyeDist  ->setText("眼距: " + formatFeature(f.eyeDistance, "eyeDistance"));
    labEyeSize  ->setText("眼大小: " + formatFeature(f.eyeSize, "eyeSize"));
    labNoseWing ->setText("鼻翼: " + formatFeature(f.noseWing, "nose"));
    labLipThick ->setText("唇厚: " + formatFeature(f.lipThickness, "lipThick"));
    labLipWidth ->setText("唇宽: " + formatFeature(f.lipWidth, "lipWidth"));
    labBrowThick->setText("眉粗: " + formatFeature(f.browThickness, "browThick"));
    labBrowDist ->setText("眉眼距: " + formatFeature(f.browDistance, "browDist"));
}

QString ScoreDisplay::formatFeature(const QString &value, const QString &category)
{
    QString color = "#a0a0a0"; // 默认灰色
    if (category == "eyeShape") {
        if (value == "圆大") color = "#ff6b9d";
        else if (value == "细长") color = "#6baaff";
    } else if (category == "eyeDistance") {
        if (value.contains("宽")) color = "#ffaa5e";
        else if (value.contains("近")) color = "#5eb8ff";
    } else if (category == "eyeSize") {
        if (value == "偏大") color = "#ff6b6b";
        else if (value == "偏小") color = "#6bc9ff";
    } else if (category == "nose") {
        if (value.contains("宽")) color = "#ffaa5e";
        else if (value.contains("窄")) color = "#5eb8ff";
    } else if (category == "lipThick") {
        if (value.contains("厚")) color = "#ff6b9d";
        else if (value.contains("薄")) color = "#6baaff";
    } else if (category == "lipWidth") {
        if (value.contains("大")) color = "#ffaa5e";
        else if (value.contains("小")) color = "#5eb8ff";
    } else if (category == "browThick") {
        if (value.contains("粗")) color = "#ffaa5e";
        else if (value.contains("细")) color = "#5eb8ff";
    } else if (category == "browDist") {
        if (value.contains("远")) color = "#ffaa5e";
        else if (value.contains("近")) color = "#5eb8ff";
    }
    return QString("<span style='color:%1;'>●</span> %2").arg(color, value);
}

void ScoreDisplay::updateScore(double sanTing, double wuYan, double symmetry,
                               double skin, double feature, double total)
{
    lab_total->setText(QString::number(total, 'f', 1));

    QString comment = getComment(total);
    QString commentColor;
    if (total >= 85)      commentColor = "#ff4d6d";
    else if (total >= 70) commentColor = "#43e97b";
    else if (total >= 55) commentColor = "#f9ca24";
    else if (total >= 40) commentColor = "#f0932b";
    else                  commentColor = "#badc58";

    lab_comment->setText("评语：" + comment);
    lab_comment->setStyleSheet(QString("font-size:16px; font-weight:bold; color:%1; padding:4px;").arg(commentColor));

    auto updateLabel = [](QLabel *lab, const QString &icon, const QString &name, double val) {
        lab->setText(QString("%1 %2  %3/20").arg(icon, name, QString::number(val, 'f', 1)));
    };
    updateLabel(lab_san, "🔺", "三庭", sanTing);
    updateLabel(lab_wu, "👁️", "五眼", wuYan);
    updateLabel(lab_sym, "🔄", "对称", symmetry);
    updateLabel(lab_skin, "✨", "皮肤", skin);
    updateLabel(lab_feat, "🎭", "协调", feature);

    auto setBar = [&](QProgressBar *bar, double val) {
        bar->setValue(static_cast<int>(val + 0.5));
        applyProgressStyle(bar, val);
    };
    setBar(bar_san, sanTing);
    setBar(bar_wu, wuYan);
    setBar(bar_sym, symmetry);
    setBar(bar_skin, skin);
    setBar(bar_feat, feature);
}

void ScoreDisplay::applyProgressStyle(QProgressBar *bar, double value)
{
    QString color;
    if (value < 8)
        color = "stop:0 #ff4b2b, stop:1 #ff416c";
    else if (value < 12)
        color = "stop:0 #f9ca24, stop:1 #f0932b";
    else if (value < 16)
        color = "stop:0 #56ab2f, stop:1 #a8e063";
    else
        color = "stop:0 #4facfe, stop:1 #00f2fe";

    bar->setStyleSheet(QString(R"(
        QProgressBar {
            border: none;
            border-radius: 8px;
            background: #3a3a3a;
        }
        QProgressBar::chunk {
            border-radius: 8px;
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0,
                                        %1);
        }
    )").arg(color));
}

QString ScoreDisplay::getComment(double total)
{
    if (total >= 85) return "颜值出众，气质绝佳 🌟";
    if (total >= 70) return "五官端正，清秀耐看 ✨";
    if (total >= 55) return "普通颜值，亲和力强 😊";
    if (total >= 40) return "五官均衡，风格自然 🍃";
    return "颜值潜力股，风格独特 💪";
}

void ScoreDisplay::clear()
{
    lab_total->setText("--");
    lab_comment->setText("评语：--");
    lab_comment->setStyleSheet("font-size:16px; font-weight:bold; color:#aaaaaa; padding:4px;");

    lab_san->setText("🔺 三庭  0/20");
    lab_wu->setText("👁️ 五眼  0/20");
    lab_sym->setText("🔄 对称  0/20");
    lab_skin->setText("✨ 皮肤  0/20");
    lab_feat->setText("🎭 协调  0/20");

    bar_san->setValue(0);
    bar_wu->setValue(0);
    bar_sym->setValue(0);
    bar_skin->setValue(0);
    bar_feat->setValue(0);

    QString defaultStyle = R"(
        QProgressBar {
            border: none;
            border-radius: 8px;
            background: #3a3a3a;
        }
        QProgressBar::chunk {
            border-radius: 8px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #56ab2f, stop:1 #a8e063);
        }
    )";
    bar_san->setStyleSheet(defaultStyle);
    bar_wu->setStyleSheet(defaultStyle);
    bar_sym->setStyleSheet(defaultStyle);
    bar_skin->setStyleSheet(defaultStyle);
    bar_feat->setStyleSheet(defaultStyle);

    // 重置五官卡片标签
    labEyeShape ->setText("眼型: --");
    labEyeDist  ->setText("眼距: --");
    labEyeSize  ->setText("眼大小: --");
    labNoseWing ->setText("鼻翼: --");
    labLipThick ->setText("唇厚: --");
    labLipWidth ->setText("唇宽: --");
    labBrowThick->setText("眉粗: --");
    labBrowDist ->setText("眉眼距: --");
}