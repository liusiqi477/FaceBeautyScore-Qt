#include "scoredisplay.h"
#include <QFont>
#include <QPalette>

ScoreDisplay::ScoreDisplay(QWidget *parent)
{
    QVBoxLayout *layout = new QVBoxLayout(parent);
    layout->setSpacing(10);
    layout->setContentsMargins(15,15,15,15);

    // 总分
    lab_total = new QLabel("总分：--");
    QFont f = lab_total->font();
    f.setPointSize(14);
    f.setBold(true);
    lab_total->setFont(f);

    // 评语
    lab_comment = new QLabel("评语：--");
    lab_comment->setStyleSheet("color:#c41e3a; font-size:13px;");

    // 进度条样式
    auto style = R"(
        QProgressBar {
            border:1px solid #ccc;
            border-radius:7px;
            background:#f0f0f0;
            height:14px;
        }
        QProgressBar::chunk {
            border-radius:7px;
            background:qlineargradient(spread:pad,x1:0,y1:0,x2:1,y2:0,stop:0 #4facfe,stop:1 #00f2fe);
        }
    )";

    // ========== 标题 Label ==========
    lab_san = new QLabel("三庭（0/40分）");
    lab_wu = new QLabel("五眼（0/20分）");
    lab_sym = new QLabel("对称（0/30分）");
    lab_skin = new QLabel("皮肤（0/5分）");
    lab_feat = new QLabel("轮廓（0/5分）");

    // ========== 进度条 ==========
    bar_san = new QProgressBar;
    bar_wu = new QProgressBar;
    bar_sym = new QProgressBar;
    bar_skin = new QProgressBar;
    bar_feat = new QProgressBar;

    bar_san->setStyleSheet(style);
    bar_wu->setStyleSheet(style);
    bar_sym->setStyleSheet(style);
    bar_skin->setStyleSheet(style);
    bar_feat->setStyleSheet(style);

    bar_san->setRange(0,40);
    bar_wu->setRange(0,20);
    bar_sym->setRange(0,30);
    bar_skin->setRange(0,5);
    bar_feat->setRange(0,5);

    // 不显示百分比
    bar_san->setTextVisible(false);
    bar_wu->setTextVisible(false);
    bar_sym->setTextVisible(false);
    bar_skin->setTextVisible(false);
    bar_feat->setTextVisible(false);

    // 布局
    layout->addWidget(lab_total);
    layout->addWidget(lab_comment);

    layout->addWidget(lab_san);
    layout->addWidget(bar_san);

    layout->addWidget(lab_wu);
    layout->addWidget(bar_wu);

    layout->addWidget(lab_sym);
    layout->addWidget(bar_sym);

    layout->addWidget(lab_skin);
    layout->addWidget(bar_skin);

    layout->addWidget(lab_feat);
    layout->addWidget(bar_feat);

    parent->setLayout(layout);
    clear();
}

void ScoreDisplay::updateScore(double sanTing, double wuYan, double symmetry,
                               double skin, double feature, double total)
{
    lab_total->setText(QString("总分：%1 分").arg(total, 0, 'f', 1));
    lab_comment->setText("评语：" + getComment(total));

    // 直接更新标题分数
    lab_san->setText(QString("三庭（%1/40分）").arg(sanTing, 0, 'f', 1));
    lab_wu->setText(QString("五眼（%1/20分）").arg(wuYan, 0, 'f', 1));
    lab_sym->setText(QString("对称（%1/30分）").arg(symmetry, 0, 'f', 1));
    lab_skin->setText(QString("皮肤（%1/5分）").arg(skin, 0, 'f', 1));
    lab_feat->setText(QString("轮廓（%1/5分）").arg(feature, 0, 'f', 1));

    bar_san->setValue(sanTing);
    bar_wu->setValue(wuYan);
    bar_sym->setValue(symmetry);
    bar_skin->setValue(skin);
    bar_feat->setValue(feature);
}

QString ScoreDisplay::getComment(double total)
{
    if(total >= 85) return "颜值出众，气质绝佳";
    else if(total >=70) return "五官端正，清秀耐看";
    else if(total >=55) return "普通颜值，亲和力强";
    else if(total >=40) return "五官均衡，风格自然";
    else return "颜值潜力股，风格独特";
}

void ScoreDisplay::clear()
{
    lab_total->setText("总分：--");
    lab_comment->setText("评语：--");

    // 重置为 0
    lab_san->setText("三庭（0/40分）");
    lab_wu->setText("五眼（0/20分）");
    lab_sym->setText("对称（0/30分）");
    lab_skin->setText("皮肤（0/5分）");
    lab_feat->setText("轮廓（0/5分）");

    bar_san->setValue(0);
    bar_wu->setValue(0);
    bar_sym->setValue(0);
    bar_skin->setValue(0);
    bar_feat->setValue(0);
}