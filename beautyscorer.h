#ifndef BEAUTYSCORER_H
#define BEAUTYSCORER_H

#include <opencv2/opencv.hpp>
#include <vector>

class BeautyScorer
{
public:
    BeautyScorer();

    // 总入口：传入对齐后的68点 + 对齐后的人脸 → 返回总分
    double calculateTotalScore(const std::vector<cv::Point>& landmarks, const cv::Mat& alignedFace);

    // 各分项得分（外部可读取）
    double sanTingScore() const;    // 三庭 0-40
    double wuYanScore() const;      // 五眼 0-20
    double symmetryScore() const;   // 对称 0-30
    double skinScore() const;       // 皮肤 0-5
    double featureScore() const;    // 五官精致 0-5

private:
    // 三庭五眼（核心）
    double calcSanTing(const std::vector<cv::Point>& lm);
    double calcWuYan(const std::vector<cv::Point>& lm);

    // 对称性
    double calcSymmetry(const std::vector<cv::Point>& lm);

    // 皮肤质量
    double calcSkin(const cv::Mat& face);

    // 五官比例
    double calcFeatures(const std::vector<cv::Point>& lm);

private:
    double m_sanTing;
    double m_wuYan;
    double m_symmetry;
    double m_skin;
    double m_feature;
};

#endif // BEAUTYSCORER_H