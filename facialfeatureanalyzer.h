#ifndef FACIALFEATUREANALYZER_H
#define FACIALFEATUREANALYZER_H

#include <QString>
#include <vector>
#include <opencv2/core.hpp>

// 五官分析结果结构体
struct FacialFeatures {
    // 眼睛
    QString eyeShape;        // "圆大" / "适中" / "细长"
    QString eyeDistance;     // "眼距较近" / "正常" / "眼距较宽"
    QString eyeSize;         // "偏小" / "适中" / "偏大" (基于脸宽)

    // 鼻子
    QString noseWing;        // "鼻翼窄" / "适中" / "鼻翼宽"

    // 嘴唇
    QString lipThickness;    // "薄唇" / "适中" / "厚唇"
    QString lipWidth;        // "嘴小" / "适中" / "嘴大"

    // 眉毛
    QString browThickness;   // "细眉" / "适中" / "粗眉"
    QString browDistance;    // "眉眼距离近" / "适中" / "眉眼距离远"

    // 综合总结
    QString summary;
};

class FacialFeatureAnalyzer {
public:
    FacialFeatureAnalyzer();

    // 核心：传入 68 个关键点，返回分析结果
    FacialFeatures analyze(const std::vector<cv::Point>& landmarks) const;

private:
    // 内部计算函数
    double eyeAspectRatio(const std::vector<cv::Point>& lm, bool leftEye) const;
    double eyeOpenHeight(const std::vector<cv::Point>& lm, bool leftEye) const;
    double faceWidth(const std::vector<cv::Point>& lm) const;
    double intercanthalWidth(const std::vector<cv::Point>& lm) const;
    double noseWingWidth(const std::vector<cv::Point>& lm) const;
    double lipHeight(const std::vector<cv::Point>& lm) const;
    double lipWidth(const std::vector<cv::Point>& lm) const;
    double browHeight(const std::vector<cv::Point>& lm, bool leftBrow) const;
    double browEyeDistance(const std::vector<cv::Point>& lm, bool leftSide) const;

    // 根据比例生成文字描述
    QString classifyEyeShape(double ratio) const;
    QString classifyEyeDistance(double ratio) const;
    QString classifyEyeSize(double ratio) const;
    QString classifyNoseWing(double ratio) const;
    QString classifyLipThickness(double ratio) const;
    QString classifyLipWidth(double ratio) const;
    QString classifyBrowThickness(double ratio) const;
    QString classifyBrowDistance(double ratio) const;
};

#endif // FACIALFEATUREANALYZER_H