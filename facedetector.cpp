#include "facedetector.h"
#include <QCoreApplication>
#include <opencv2/imgproc.hpp>

FaceDetector::FaceDetector()
{
    QString path = QCoreApplication::applicationDirPath() +
                   "/shape_predictor_68_face_landmarks.dat";
    dlib::deserialize(path.toLocal8Bit().data()) >> sp;
    detector = dlib::get_frontal_face_detector();
}

void FaceDetector::clear()
{
    m_faceCount = 0;
    m_rawLandmarks.clear();
    m_alignedLandmarks.clear();
    m_alignedFace.release();
}

bool FaceDetector::detect(const cv::Mat& bgrImage)
{
    if (bgrImage.empty()) return false;

    // 重置状态
    clear();

    // 转换为 RGB 供 dlib 使用
    cv::Mat rgbImage;
    cv::cvtColor(bgrImage, rgbImage, cv::COLOR_BGR2RGB);
    dlib::cv_image<dlib::rgb_pixel> dlibImg(rgbImage);

    // 检测所有人脸
    std::vector<dlib::rectangle> faces = detector(dlibImg);
    m_faceCount = static_cast<int>(faces.size());
    if (faces.empty())
        return false;

    // 只处理最大人脸（符合你“自动选取最大人脸”的需求）
    dlib::rectangle bestFace = getLargestFace(faces);
    dlib::full_object_detection shape = sp(dlibImg, bestFace);

    // 保存原始坐标（基于 BGR 图像尺寸，可用于在原图上画点）
    m_rawLandmarks.reserve(68);
    for (int i = 0; i < 68; ++i) {
        m_rawLandmarks.emplace_back(shape.part(i).x(), shape.part(i).y());
    }

    //计算对齐正脸 + 对齐后的关键点
    return computeAlignment(bgrImage, m_rawLandmarks, m_alignedFace, m_alignedLandmarks);
}

dlib::rectangle FaceDetector::getLargestFace(const std::vector<dlib::rectangle>& faces) const
{
    int maxArea = 0;
    dlib::rectangle largest;
    for (const auto& f : faces) {
        int area = f.width() * f.height();
        if (area > maxArea) {
            maxArea = area;
            largest = f;
        }
    }
    return largest;
}

bool FaceDetector::computeAlignment(const cv::Mat& srcBgr,
                                    const std::vector<cv::Point>& srcLandmarks,
                                    cv::Mat& outAlignedFace,
                                    std::vector<cv::Point>& outAlignedLandmarks) const
{
    if (srcLandmarks.size() < 68)
        return false;

    // 提取双眼中心 + 鼻尖（原始图坐标）
    cv::Point2f leftEye  = (srcLandmarks[36] + srcLandmarks[39]) * 0.5f;
    cv::Point2f rightEye = (srcLandmarks[42] + srcLandmarks[45]) * 0.5f;
    cv::Point2f noseTip  = srcLandmarks[30];

    // 快速偏航角过滤（防止侧脸过于严重）
    double noseMidX = srcLandmarks[27].x;
    double leftDist  = std::abs(leftEye.x - noseMidX);
    double rightDist = std::abs(rightEye.x - noseMidX);
    const double minRatio = 0.55;
    const double maxRatio = 1.0 / minRatio;
    double ratio = (rightDist > 1e-6) ? (leftDist / rightDist) : 1.0;
    if (ratio < minRatio || ratio > maxRatio) {
        return false;   // 偏转太大，不生成对齐结果（但原始点仍有效）
    }

    // 目标点（相似变换，保持原始比例）
    cv::Point2f targetLeftEye (64.0f,  80.0f);
    cv::Point2f targetRightEye(192.0f, 80.0f);
    cv::Point2f targetNose    (128.0f, 145.0f);

    std::vector<cv::Point2f> srcPts = {leftEye, rightEye, noseTip};
    std::vector<cv::Point2f> dstPts = {targetLeftEye, targetRightEye, targetNose};

    cv::Mat trans = cv::estimateAffinePartial2D(srcPts, dstPts);
    if (trans.empty())
        return false;

    // 生成对齐后的正脸图像
    cv::warpAffine(srcBgr, outAlignedFace, trans, cv::Size(256, 256),
                   cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0,0,0));

    // 将原始关键点变换到对齐后的坐标
    outAlignedLandmarks.clear();
    outAlignedLandmarks.reserve(srcLandmarks.size());
    for (const auto& p : srcLandmarks) {
        cv::Point2f pt(p.x, p.y);
        float x = trans.at<double>(0,0) * pt.x + trans.at<double>(0,1) * pt.y + trans.at<double>(0,2);
        float y = trans.at<double>(1,0) * pt.x + trans.at<double>(1,1) * pt.y + trans.at<double>(1,2);
        outAlignedLandmarks.emplace_back(cvRound(x), cvRound(y));
    }
    return true;
}

void FaceDetector::drawLandmarks(cv::Mat& image, const std::vector<cv::Point>& points,
                                 cv::Scalar color, int radius)
{
    for (const auto& p : points) {
        cv::circle(image, p, radius, color, -1);
    }
}