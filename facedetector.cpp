#include "facedetector.h"
#include <QCoreApplication>

FaceDetector::FaceDetector()
{
    // 加载模型
    QString path = QCoreApplication::applicationDirPath() + "/shape_predictor_68_face_landmarks.dat";
    dlib::deserialize(path.toLocal8Bit().data()) >> sp;
    detector = dlib::get_frontal_face_detector();
}
int FaceDetector::getFaceCount() const{ return m_faceCount; }
// ✅ 获取68点
std::vector<cv::Point> FaceDetector::getLandmarks() const
{
    return m_landmarks;
}

cv::Mat FaceDetector::getAlignedFace() const
{
    return m_alignedFace;
}

// 【修复】这里必须完全一致
std::vector<cv::Point> FaceDetector::getAlignedLandmarks() const
{
    return m_alignedLandmarks;
}


dlib::rectangle FaceDetector::getLargestFace(const std::vector<dlib::rectangle>& faces)
{
    int maxArea = 0;
    dlib::rectangle largest;
    for(auto& f : faces){
        int area = f.width() * f.height();
        if(area > maxArea){
            maxArea = area;
            largest = f;
        }
    }
    return largest;
}


bool FaceDetector::alignFace(const cv::Mat &inputImg)
{
    if (m_landmarks.size() < 68)
        return false;

    // 1. 提取参考点
    cv::Point2f leftEye  = (m_landmarks[36] + m_landmarks[39]) * 0.5f;
    cv::Point2f rightEye = (m_landmarks[42] + m_landmarks[45]) * 0.5f;
    cv::Point2f noseTip  = m_landmarks[30];

    // 2. 粗略偏航角检测（侧脸程度）
    //    用鼻梁上部点27作为中线参考（比30更稳定）
    double noseMidX = m_landmarks[27].x;
    double leftDist  = std::abs(leftEye.x - noseMidX);
    double rightDist = std::abs(rightEye.x - noseMidX);

    // 允许的左右距离比值范围，可根据实际调整
    const double minRatio = 0.55;
    const double maxRatio = 1.0 / minRatio;  // 约1.82
    double ratio = (rightDist > 1e-6) ? (leftDist / rightDist) : 1.0;
    if (ratio < minRatio || ratio > maxRatio) {
        // 偏离过大，拒绝处理，外部可提示用户
        return false;
    }

    // 3. 定义相似变换的目标点
    //    这里只追求双眼水平且瞳距归一化，鼻尖用于辅助垂直定位
    //    由于是相似变换，目标点无需与源点构成完全相同比例，算法会最小化误差。
    cv::Point2f targetLeftEye (64.0f,  80.0f);
    cv::Point2f targetRightEye(192.0f, 80.0f);
    cv::Point2f targetNose    (128.0f, 145.0f);

    std::vector<cv::Point2f> srcPts = {leftEye, rightEye, noseTip};
    std::vector<cv::Point2f> dstPts = {targetLeftEye, targetRightEye, targetNose};

    // 4. 估计相似变换矩阵（只旋转+平移+均匀缩放）
    cv::Mat trans = cv::estimateAffinePartial2D(srcPts, dstPts);
    if (trans.empty())
        return false;   // 估计失败

    // 5. 生成对齐后的正脸图像
    cv::warpAffine(inputImg, m_alignedFace, trans, cv::Size(256, 256),
                   cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0,0,0));

    // 6. 将68个关键点变换到对齐后的坐标
    m_alignedLandmarks.clear();
    m_alignedLandmarks.reserve(m_landmarks.size());
    for (const auto &p : m_landmarks) {
        cv::Point2f pt(p.x, p.y);
        // 手动应用 2x3 仿射矩阵
        float x = trans.at<double>(0,0) * pt.x + trans.at<double>(0,1) * pt.y + trans.at<double>(0,2);
        float y = trans.at<double>(1,0) * pt.x + trans.at<double>(1,1) * pt.y + trans.at<double>(1,2);
        m_alignedLandmarks.push_back(cv::Point(cvRound(x), cvRound(y)));
    }

    return true;
}


QImage FaceDetector::detect68Points(const QString &imagePath)
{
    // 1. 读取图片
    cv::Mat img = cv::imread(imagePath.toLocal8Bit().toStdString());
    if (img.empty()) return QImage();

    // 2. 转换格式 + 检测
    dlib::cv_image<dlib::rgb_pixel> dlib_img(img);
    std::vector<dlib::rectangle> faces = detector(dlib_img); // 强制指定 dlib::rectangle


    // 清空数据
    m_landmarks.clear();
    m_alignedLandmarks.clear();
    m_alignedFace.release();

    // 3. 检测68点并存起来
    for (auto &face : faces) {
        dlib::full_object_detection shape = sp(dlib_img, face);
        for (int i = 0; i < 68; ++i) {
            m_landmarks.emplace_back(shape.part(i).x(), shape.part(i).y());
        }
        alignFace(img); // 对齐（不影响原图）
    }

    // ===================================================================
    // ✅ 【核心】在原图上画出68个点，返回给Qt显示
    // ===================================================================
    cv::Mat showImg = img.clone();
    for (cv::Point p : m_landmarks) {
        cv::circle(showImg, p, 3, cv::Scalar(0, 255, 0), -1); // 绿色圆点，稍大一点更清楚
    }

    // 转Qt图片
    cv::cvtColor(showImg, showImg, cv::COLOR_BGR2RGB);
    return QImage(showImg.data, showImg.cols, showImg.rows, showImg.step, QImage::Format_RGB888).copy();
}
QImage FaceDetector::detect68Points_Mat(cv::Mat img)
{
    if (img.empty()) return QImage();

    // ✅【修复】摄像头是 BGR，必须转成 RGB 给 dlib
    cv::Mat rgb;
    cv::cvtColor(img, rgb, cv::COLOR_BGR2RGB);

    dlib::cv_image<dlib::rgb_pixel> dlib_img(rgb);

    std::vector<dlib::rectangle> faces = detector(dlib_img);
    m_faceCount = faces.size();

    m_landmarks.clear();
    m_alignedLandmarks.clear();
    m_alignedFace.release();

    if (faces.empty()) return QImage();

    auto best = getLargestFace(faces);
    auto shape = sp(dlib_img, best);

    for (int i = 0; i < 68; ++i) {
        m_landmarks.emplace_back(shape.part(i).x(), shape.part(i).y());
    }

    alignFace(img);
    return QImage();
}
