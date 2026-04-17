#ifndef SCOREDISPLAY_H
#define SCOREDISPLAY_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QString>

class ScoreDisplay
{
public:
    // 传入父widget（放分数的地方）
    ScoreDisplay(QWidget *parent);

    // 更新所有分数 + 评语
    void updateScore(double sanTing, double wuYan, double symmetry,
                     double skin, double feature, double total);

    // 清空重置
    void clear();

private:
    // 生成评语
    QString getComment(double total);

private:

    QLabel *lab_total;
    QLabel *lab_comment;

    QLabel *lab_san;
    QLabel *lab_wu;
    QLabel *lab_sym;
    QLabel *lab_skin;
    QLabel *lab_feat;

    QProgressBar *bar_san;
    QProgressBar *bar_wu;
    QProgressBar *bar_sym;
    QProgressBar *bar_skin;
    QProgressBar *bar_feat;
};

#endif // SCOREDISPLAY_H