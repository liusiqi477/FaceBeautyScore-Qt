#include "facialfeatureanalyzer.h"
#include <cmath>

FacialFeatureAnalyzer::FacialFeatureAnalyzer() {}

// ========== 主分析函数 ==========
FacialFeatures FacialFeatureAnalyzer::analyze(const std::vector<cv::Point>& lm) const {
    if (lm.size() < 68) return FacialFeatures();

    FacialFeatures f;

    // 1. 眼睛形状 (眼裂高宽比)
    double leftEyeAR  = eyeAspectRatio(lm, true);
    double rightEyeAR = eyeAspectRatio(lm, false);
    double avgEyeAR   = (leftEyeAR + rightEyeAR) / 2.0;
    f.eyeShape = classifyEyeShape(avgEyeAR);

    // 2. 眼距 (内眼角间距 / 脸宽)
    double inter = intercanthalWidth(lm);
    double fw = faceWidth(lm);
    double eyeDistRatio = inter / fw;
    f.eyeDistance = classifyEyeDistance(eyeDistRatio);

    // 3. 眼睛大小 (眼裂高度 / 脸高) 简化用脸宽
    double eyeH = (eyeOpenHeight(lm, true) + eyeOpenHeight(lm, false)) / 2.0;
    double eyeSizeRatio = eyeH / fw;
    f.eyeSize = classifyEyeSize(eyeSizeRatio);

    // 4. 鼻翼宽度
    double noseW = noseWingWidth(lm);
    double noseRatio = noseW / fw;
    f.noseWing = classifyNoseWing(noseRatio);

    // 5. 嘴唇厚薄 (唇高 / 唇宽)
    double lpH = lipHeight(lm);
    double lpW = lipWidth(lm);
    double lipRatio = lpH / lpW;
    f.lipThickness = classifyLipThickness(lipRatio);

    // 6. 嘴唇宽度 (唇宽 / 脸宽)
    double lipWRatio = lpW / fw;
    f.lipWidth = classifyLipWidth(lipWRatio);

    // 7. 眉毛粗细 (眉上下缘平均距离)
    double leftBrowH = browHeight(lm, true);
    double rightBrowH = browHeight(lm, false);
    double avgBrowH = (leftBrowH + rightBrowH) / 2.0;
    double browThickRatio = avgBrowH / fw;
    f.browThickness = classifyBrowThickness(browThickRatio);

    // 8. 眉眼距离 (眉下缘到眼上缘)
    double leftBrowEye = browEyeDistance(lm, true);
    double rightBrowEye = browEyeDistance(lm, false);
    double avgBrowEye = (leftBrowEye + rightBrowEye) / 2.0;
    double browEyeRatio = avgBrowEye / fw;
    f.browDistance = classifyBrowDistance(browEyeRatio);

    // 9. 生成综合总结
    f.summary = QString("眼型%1，%2，眼睛%3；鼻翼%4；%5，%6；眉毛%7，%8。")
                    .arg(f.eyeShape, f.eyeDistance, f.eyeSize,
                         f.noseWing, f.lipThickness, f.lipWidth,
                         f.browThickness, f.browDistance);

    return f;
}

// ========== 测量函数 ==========
double FacialFeatureAnalyzer::eyeAspectRatio(const std::vector<cv::Point>& lm, bool leftEye) const {
    int base = leftEye ? 36 : 42;
    double h1 = std::abs(lm[base+1].y - lm[base+5].y);
    double h2 = std::abs(lm[base+2].y - lm[base+4].y);
    double w  = std::abs(lm[base+3].x - lm[base+0].x);
    if (w < 1) return 0;
    return ((h1 + h2) / 2.0) / w;
}

double FacialFeatureAnalyzer::eyeOpenHeight(const std::vector<cv::Point>& lm, bool leftEye) const {
    int base = leftEye ? 36 : 42;
    double h1 = std::abs(lm[base+1].y - lm[base+5].y);
    double h2 = std::abs(lm[base+2].y - lm[base+4].y);
    return (h1 + h2) / 2.0;
}

double FacialFeatureAnalyzer::faceWidth(const std::vector<cv::Point>& lm) const {
    return std::abs(lm[16].x - lm[0].x);
}

double FacialFeatureAnalyzer::intercanthalWidth(const std::vector<cv::Point>& lm) const {
    return std::abs(lm[39].x - lm[42].x);
}

double FacialFeatureAnalyzer::noseWingWidth(const std::vector<cv::Point>& lm) const {
    return std::abs(lm[35].x - lm[31].x);
}

double FacialFeatureAnalyzer::lipHeight(const std::vector<cv::Point>& lm) const {
    return std::abs(lm[51].y - lm[57].y);
}

double FacialFeatureAnalyzer::lipWidth(const std::vector<cv::Point>& lm) const {
    return std::abs(lm[54].x - lm[48].x);
}

double FacialFeatureAnalyzer::browHeight(const std::vector<cv::Point>& lm, bool leftBrow) const {
    int base = leftBrow ? 17 : 22;
    // 眉最高点与最低点的垂直距离 (近似用 19 vs 18 或 24 vs 23)
    double top = (lm[base+2].y < lm[base+1].y) ? lm[base+2].y : lm[base+1].y;
    double bottom = lm[base].y;
    return std::abs(bottom - top);
}

double FacialFeatureAnalyzer::browEyeDistance(const std::vector<cv::Point>& lm, bool leftSide) const {
    // 眉下缘 (lm[19] 或 lm[24]) 到 眼上缘中点 (lm[37] 或 lm[43])
    int browIdx = leftSide ? 19 : 24;
    int eyeTopIdx = leftSide ? 37 : 43;
    return std::abs(lm[eyeTopIdx].y - lm[browIdx].y);
}

// ========== 分类函数 (阈值可自行调优) ==========
QString FacialFeatureAnalyzer::classifyEyeShape(double ratio) const {
    if (ratio < 0.22) return "细长";
    if (ratio > 0.32) return "圆大";
    return "适中";
}

QString FacialFeatureAnalyzer::classifyEyeDistance(double ratio) const {
    // 内眼角间距/脸宽 通常 0.22~0.28
    if (ratio < 0.21) return "较近";
    if (ratio > 0.27) return "较宽";
    return "正常";
}

QString FacialFeatureAnalyzer::classifyEyeSize(double ratio) const {
    // 眼高/脸宽 通常 0.035~0.05
    if (ratio < 0.035) return "偏小";
    if (ratio > 0.05) return "偏大";
    return "适中";
}

QString FacialFeatureAnalyzer::classifyNoseWing(double ratio) const {
    // 鼻翼宽/脸宽 约 0.22~0.28
    if (ratio < 0.21) return "窄";
    if (ratio > 0.28) return "宽";
    return "适中";
}

QString FacialFeatureAnalyzer::classifyLipThickness(double ratio) const {
    // 唇高/唇宽 约 0.3~0.45
    if (ratio < 0.32) return "薄唇";
    if (ratio > 0.45) return "厚唇";
    return "适中";
}

QString FacialFeatureAnalyzer::classifyLipWidth(double ratio) const {
    // 唇宽/脸宽 约 0.3~0.4
    if (ratio < 0.28) return "嘴小";
    if (ratio > 0.40) return "嘴大";
    return "适中";
}

QString FacialFeatureAnalyzer::classifyBrowThickness(double ratio) const {
    // 眉高/脸宽 约 0.03~0.06
    if (ratio < 0.03) return "细眉";
    if (ratio > 0.06) return "粗眉";
    return "适中";
}

QString FacialFeatureAnalyzer::classifyBrowDistance(double ratio) const {
    // 眉眼距/脸宽 约 0.06~0.09
    if (ratio < 0.05) return "近";
    if (ratio > 0.09) return "远";
    return "适中";
}