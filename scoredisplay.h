#ifndef SCOREDISPLAY_H
#define SCOREDISPLAY_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>

// 前向声明
struct FacialFeatures;

class ScoreDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit ScoreDisplay(QWidget *parent = nullptr);

    void updateScore(double sanTing, double wuYan, double symmetry,
                     double skin, double feature, double total);

    // 核心：显示五官分析 (基于 FacialFeatures 结构体)
    void setFacialFeatures(const FacialFeatures &f);

    void clear();

private:
    void setupFeatureCard();   // 创建卡片布局
    QString formatFeature(const QString &value, const QString &category);
    QString getComment(double total);
    void applyProgressStyle(QProgressBar *bar, double value);

    // 原有控件
    QLabel *lab_total;
    QLabel *lab_comment;

    QLabel *lab_san, *lab_wu, *lab_sym, *lab_skin, *lab_feat;
    QProgressBar *bar_san, *bar_wu, *bar_sym, *bar_skin, *bar_feat;

    // 五官卡片相关
    QFrame *featureCard;
    QLabel *labEyeShape, *labEyeDist, *labEyeSize;
    QLabel *labNoseWing;          // 鼻翼
    QLabel *labLipThick;          // 嘴唇厚薄
    QLabel *labLipWidth;          // 嘴唇宽度
    QLabel *labBrowThick;         // 眉毛粗细
    QLabel *labBrowDist;          // 眉眼距离
};

#endif