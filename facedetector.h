#ifndef FACEDETECTOR_H
#define FACEDETECTOR_H

#include <QImage>
#include <opencv2/opencv.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/shape_predictor.h>
#include <dlib/opencv.h>
#include <vector>
#include <dlib/image_processing.h>
#include <QString>


class FaceDetector
{
public:
    FaceDetector();

    // 核心检测：传入 BGR 图像，检测最大人脸，保存原始点和对齐结果
    bool detect(const cv::Mat& bgrImage);

    // 获取检测到的人脸数量（在 detect 后有效）
    int getFaceCount() const { return m_faceCount; }

    // 获取原始图像坐标下的68个关键点（用于在原图上绘制）
    std::vector<cv::Point> getLandmarks() const { return m_rawLandmarks; }

    // 获取对齐后的图像 (256x256 正脸)
    cv::Mat getAlignedFace() const { return m_alignedFace; }

    // 获取对齐后的68个关键点（保持比例，用于评分）
    std::vector<cv::Point> getAlignedLandmarks() const { return m_alignedLandmarks; }

    // 工具：在图像上绘制关键点
    static void drawLandmarks(cv::Mat& image, const std::vector<cv::Point>& points,
                              cv::Scalar color = cv::Scalar(0, 255, 0), int radius = 3);

private:
    // 重置所有检测结果
    void clear();

    // 从 faces 中选择最大的一张脸
    dlib::rectangle getLargestFace(const std::vector<dlib::rectangle>& faces) const;

    // 对齐计算：传入原始图像和原始关键点，输出对齐图像和对齐关键点
    bool computeAlignment(const cv::Mat& srcBgr,
                          const std::vector<cv::Point>& srcLandmarks,
                          cv::Mat& outAlignedFace,
                          std::vector<cv::Point>& outAlignedLandmarks) const;

    dlib::shape_predictor sp;
    dlib::frontal_face_detector detector;

    // 最新一次检测的结果
    int m_faceCount = 0;
    std::vector<cv::Point> m_rawLandmarks;        // 原始坐标（在原图上）
    std::vector<cv::Point> m_alignedLandmarks;    // 对齐后的坐标
    cv::Mat m_alignedFace;                        // 对齐后的正脸图
};

#endif // FACEDETECTOR_H