#include "facedetector.h"
#include <QCoreApplication>

FaceDetector::FaceDetector()
{
    // 加载模型
    QString path = QCoreApplication::applicationDirPath() + "/shape_predictor_68_face_landmarks.dat";
    dlib::deserialize(path.toLocal8Bit().data()) >> sp;
    detector = dlib::get_frontal_face_detector();
}

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




void FaceDetector::alignFace(const cv::Mat &inputImg)
{
    if (m_landmarks.size() < 68) return;

    // 标准正脸目标点（256x256）
    cv::Point2f target[3] = {
        cv::Point2f(64, 80),   // 左眼中心
        cv::Point2f(192, 80),  // 右眼中心
        cv::Point2f(128, 145)  // 鼻尖
    };

    // 从68点取出双眼+鼻尖
    cv::Point2f leftEye = (m_landmarks[36] + m_landmarks[39]) * 0.5f;
    cv::Point2f rightEye = (m_landmarks[42] + m_landmarks[45]) * 0.5f;
    cv::Point2f nose = m_landmarks[30];

    cv::Point2f src[3] = {rightEye, leftEye, nose};

    // 计算仿射变换矩阵
    cv::Mat trans = cv::getAffineTransform(src, target);

    // 对齐得到标准正脸
    cv::warpAffine(inputImg, m_alignedFace, trans, cv::Size(256, 256));

    // 【关键】把68个点也变换到对齐后的坐标
    m_alignedLandmarks.clear();
    for (auto &p : m_landmarks) {
        cv::Point np;
        np.x = cvRound(p.x * trans.at<double>(0,0) + p.y * trans.at<double>(0,1) + trans.at<double>(0,2));
        np.y = cvRound(p.x * trans.at<double>(1,0) + p.y * trans.at<double>(1,1) + trans.at<double>(1,2));
        m_alignedLandmarks.push_back(np);
    }
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