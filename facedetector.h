#ifndef FACEDETECTOR_H
#define FACEDETECTOR_H

#include <QImage>
#include <opencv2/opencv.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/shape_predictor.h>
#include <dlib/opencv.h>
#include <vector>


// 注意：不要写 using namespace dlib; 或 using namespace cv;
// 避免命名空间冲突！

class FaceDetector
{
public:
    FaceDetector();
    QImage detect68Points(const QString &imagePath);

    // ✅【新增】获取68个关键点坐标，原始点用于对齐！！！！！
    std::vector<cv::Point> getLandmarks() const;

    cv::Mat getAlignedFace() const; // 【新增】获取对齐后的人脸
    std::vector<cv::Point> getAlignedLandmarks() const; // 【新增】获取对齐后的68点
private:
    // 【新增】人脸对齐核心函数
    void alignFace(const cv::Mat &inputImg);
    dlib::frontal_face_detector detector;
    dlib::shape_predictor sp;
    // ✅【关键】用vector保存68个点坐标
    std::vector<cv::Point> m_landmarks;

    // 【新增】对齐后的数据
    cv::Mat m_alignedFace;
    std::vector<cv::Point> m_alignedLandmarks;
};

#endif // FACEDETECTOR_H