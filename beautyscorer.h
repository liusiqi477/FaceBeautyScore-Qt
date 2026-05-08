#ifndef BEAUTYSCORER_H
#define BEAUTYSCORER_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <opencv2/imgproc.hpp>


class BeautyScorer
{
public:
    BeautyScorer();

    // 总入口：传入对齐后的68点 + 对齐后的人脸 → 返回总分
    double calculateTotalScore(const std::vector<cv::Point>& landmarks, const cv::Mat& alignedFace);
    double calculateTotalScore(const std::vector<cv::Point>& alignedLandmarks,
                               const cv::Mat& alignedFace,
                               const std::vector<cv::Point>& originalLandmarks,
                               const cv::Mat& originalFrame);
    // 各分项得分（外部可读取）
    double sanTingScore() const;    // 三庭 0-20
    double wuYanScore() const;      // 五眼 0-20
    double symmetryScore() const;   // 对称 0-20
    double skinScore() const;       // 皮肤 0-20
    double featureProportionsScore() const;    // 五官精致 0-20

private:
    // 三庭五眼（核心）
    double calcSanTing(const std::vector<cv::Point>& lm);
    double calcWuYan(const std::vector<cv::Point>& lm);

    // 对称性
    double calcSymmetry(const std::vector<cv::Point>& lm);

    // 皮肤质量
    double calcSkin(const cv::Mat& face,const std::vector<cv::Point>& landmarks68);
    // 五官比例
    double calcFeatureProportions(const std::vector<cv::Point>& lm);





private:
    double m_sanTing;
    double m_wuYan;
    double m_symmetry;
    double m_skin;
    double m_featureProportions;
};

#endif // BEAUTYSCORER_H