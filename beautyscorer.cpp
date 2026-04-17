#include "beautyscorer.h"
#include <cmath>
#include <QDebug>

BeautyScorer::BeautyScorer()
{
    m_sanTing = 0;
    m_wuYan = 0;
    m_symmetry = 0;
    m_skin = 0;
    m_feature = 0;
}

double BeautyScorer::calculateTotalScore(const std::vector<cv::Point>& landmarks, const cv::Mat& alignedFace)
{
    if (landmarks.size() < 68 || alignedFace.empty())
        return 0;

    m_sanTing    = calcSanTing(landmarks);
    m_wuYan      = calcWuYan(landmarks);
    m_symmetry   = calcSymmetry(landmarks);
    m_skin       = calcSkin(alignedFace);
    m_feature    = calcFeatures(landmarks);

    return m_sanTing + m_wuYan + m_symmetry + m_skin + m_feature;
}

// 三庭（0-40） 发际线→眉骨→鼻底→下巴 1:1:1
// ==============================
double BeautyScorer::calcSanTing(const std::vector<cv::Point>& lm)
{
    int hair = lm[27].y - (lm[27].y - lm[24].y)*2; // 估算发际线
    int brow = lm[27].y;
    int noseBottom = lm[33].y;
    int chin = lm[8].y;

    double t1 = abs(brow - hair);
    double t2 = abs(noseBottom - brow);
    double t3 = abs(chin - noseBottom);

    double avg = (t1 + t2 + t3) / 3;
    double err = (abs(t1-avg)+abs(t2-avg)+abs(t3-avg))/3;

    double score = 40 - (err / avg)*40;
    return qMax(0.0, score);
}

// ==============================
// 五眼（0-20）脸宽 ≈ 5倍眼距
// ==============================
double BeautyScorer::calcWuYan(const std::vector<cv::Point>& lm)
{
    int left = lm[0].x;
    int right = lm[16].x;
    int eyeW = lm[39].x - lm[36].x;
    double faceW = right-left;

    double ratio = faceW / (eyeW*5.0);
    double score = 20 - abs(1-ratio)*20;
    return qMax(0.0, score);
}

// ==============================
// 对称性（0-30）左右偏差越小分越高
// ==============================
double BeautyScorer::calcSymmetry(const std::vector<cv::Point>& lm)
{
    int midX = (lm[0].x + lm[16].x)/2;
    double totalErr = 0;
    int pairs[][2] = {{36,45},{39,42},{48,54},{57,57},{0,16},{1,15},{2,14}};
    int cnt = sizeof(pairs)/sizeof(pairs[0]);

    for(int i=0;i<cnt;++i) {
        int l = pairs[i][0];
        int r = pairs[i][1];
        int lx = lm[l].x;
        int rx = lm[r].x;
        totalErr += abs( (midX - lx) - (rx - midX) );
    }

    double score = 30 - (totalErr/cnt)/3.0;
    return qMax(0.0, score);
}

// ==============================
// 皮肤平滑度（0-5）
// ==============================
double BeautyScorer::calcSkin(const cv::Mat& face)
{
    cv::Mat gray, laplacian;
    cv::cvtColor(face, gray, cv::COLOR_BGR2GRAY);
    cv::Laplacian(gray, laplacian, CV_64F);
    cv::Scalar mean, stddev;
    cv::meanStdDev(laplacian, mean, stddev);
    double texture = stddev.val[0];
    double score = 5 - (texture/20);
    return qMax(0.0, score);
}


// 面部轮廓协调度 0~5 分（客观无审美，无警告版）
double BeautyScorer::calcFeatures(const std::vector<cv::Point>& lm)
{
    // 左右脸颊高度差
    double leftCheek = fabs(lm[4].y - lm[8].y);
    double rightCheek = fabs(lm[12].y - lm[8].y);
    double cheekErr = fabs(leftCheek - rightCheek);

    // 下巴左右对称性差异
    double jawLeft = fabs(lm[8].x - lm[4].x);
    double jawRight = fabs(lm[12].x - lm[8].x);
    double jawErr = fabs(jawLeft - jawRight);

    // 眉心到脸颊两侧均衡度
    double midX = (lm[0].x + lm[16].x) / 2.0;
    double midErr = fabs((midX - lm[2].x) - (lm[14].x - midX));

    // 总误差越小，协调度越高
    double totalErr = (cheekErr + jawErr + midErr) * 0.08;

    // 分数 0~5
    double score = 5.0 - totalErr;
    return qBound(0.0, score, 5.0);
}

double BeautyScorer::sanTingScore()    const { return m_sanTing; }
double BeautyScorer::wuYanScore()      const { return m_wuYan; }
double BeautyScorer::symmetryScore()   const { return m_symmetry; }
double BeautyScorer::skinScore()       const { return m_skin; }
double BeautyScorer::featureScore()    const { return m_feature; }