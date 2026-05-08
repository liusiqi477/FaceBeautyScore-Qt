#include "beautyscorer.h"
#include <cmath>
#include <QDebug>

BeautyScorer::BeautyScorer()
{
    m_sanTing = 0;
    m_wuYan = 0;
    m_symmetry = 0;
    m_skin = 0;
    m_featureProportions = 0;
}

double BeautyScorer::calculateTotalScore(const std::vector<cv::Point>& landmarks, const cv::Mat& alignedFace)
{
    if (landmarks.size() < 68 || alignedFace.empty())
        return 0;

    m_sanTing    = calcSanTing(landmarks);
    m_wuYan      = calcWuYan(landmarks);
    m_symmetry   = calcSymmetry(landmarks);
    m_skin = calcSkin(alignedFace, landmarks);
    m_featureProportions    = calcFeatureProportions(landmarks);
    return m_sanTing + m_wuYan + m_symmetry + m_skin + m_featureProportions;
}

double BeautyScorer::calculateTotalScore(const std::vector<cv::Point>& alignedLandmarks,
                                         const cv::Mat& alignedFace,
                                         const std::vector<cv::Point>& originalLandmarks,
                                         const cv::Mat& originalFrame)
{
    //只要对齐数据不足就和原来一致
    if (alignedLandmarks.size() < 68 || alignedFace.empty())
        return 0;

    // 五官、三庭、对称、特征比：继续用对齐后的点
    m_sanTing    = calcSanTing(alignedLandmarks);
    m_wuYan      = calcWuYan(alignedLandmarks);
    m_symmetry   = calcSymmetry(alignedLandmarks);
    m_featureProportions = calcFeatureProportions(alignedLandmarks);

    // 皮肤纹理：用原始大图和原始关键点
    if (originalLandmarks.size() == 68 && !originalFrame.empty())
        m_skin = calcSkin(originalFrame, originalLandmarks);
    else
        m_skin = calcSkin(alignedFace, alignedLandmarks);  // 极少数情况下的备用

    return m_sanTing + m_wuYan + m_symmetry + m_skin + m_featureProportions;
}

// 庭衡，中庭：下庭为1：1
double BeautyScorer::calcSanTing(const std::vector<cv::Point>& lm)
{
    // 使用稳定关键点
    //    眉毛内端中点作为中庭上界
    double brow_y = (lm[21].y + lm[22].y) / 2.0;

    //    鼻基底 (鼻中隔下点)
    double nose_base_y = lm[33].y;

    //    下巴最低点
    double chin_y = lm[8].y;

    // 计算中庭和下庭高度
    double middle = nose_base_y - brow_y;
    double lower  = chin_y - nose_base_y;

    // 无效保护
    if (middle <= 0 || lower <= 0)
        return 0.0;

    // 中庭与下庭的理想比例为 1:1
    double ratio = middle / lower;

    // 非线性评分 (高斯衰减)
    double tol = 0.5;   // 容忍度，越大分数越平缓
    double diff = (ratio - 1.0) / tol;
    double score = 20.0 * exp(-diff * diff);

    // 限制在 [0, 20]
    return std::max(0.0, std::min(score, 20.0));
}

// 五眼,脸宽 ≈ 5倍眼距
double BeautyScorer::calcWuYan(const std::vector<cv::Point>& lm)
{
    // 提取关键点
    double left_face  = lm[0].x;
    double right_face = lm[16].x;

    double left_eye_outer = lm[36].x;
    double left_eye_inner = lm[39].x;
    double right_eye_inner = lm[42].x;
    double right_eye_outer = lm[45].x;

    // 计算各段宽度，取绝对值以防索引镜像
    double left_margin  = std::abs(left_eye_outer - left_face);
    double left_eye_w   = std::abs(left_eye_inner - left_eye_outer);
    double intercanth   = std::abs(right_eye_inner - left_eye_inner);
    double right_eye_w  = std::abs(right_eye_outer - right_eye_inner);
    double right_margin = std::abs(right_face - right_eye_outer);

    // 平均眼宽作为基准
    double eye_avg = (left_eye_w + right_eye_w) / 2.0;
    if (eye_avg < 1e-6) return 0.0;   // 防止除零

    // 计算五段与基准眼宽的比值
    std::vector<double> ratios = {
        left_margin / eye_avg,
        left_eye_w  / eye_avg,
        intercanth  / eye_avg,
        right_eye_w / eye_avg,
        right_margin / eye_avg
    };

    // 评分：每段满分 4 分，总分 20
    //    理想 ratio=1.0，在 [0.9, 1.1] 内得满分，超出后线性扣分到 0
    double total_score = 0.0;
    for (double r : ratios) {
        double diff = std::abs(r - 1.0);
        double seg_score = 4.0;
        if (diff > 0.1) {
            // 超出 0.1 后开始扣分，到 diff=0.5 时扣为 0
            seg_score = 4.0 * std::max(0.0, 1.0 - (diff - 0.1) / 0.4);
        }
        total_score += seg_score;
    }

    return std::max(0.0, std::min(total_score, 20.0));
}

//
double BeautyScorer::calcSymmetry(const std::vector<cv::Point>& lm)
{
    //  计算稳健中轴线
    // 使用不易变形的鼻梁点 (27~30) 的 X 均值，比轮廓点稳定得多
    double midX = 0.0;
    for (int i = 27; i <= 30; ++i)
        midX += lm[i].x;
    midX /= 4.0;



    //  归一化基准：双眼外眼角间距
    double ipd = std::abs(lm[45].x - lm[36].x);
    if (ipd < 1.0) return 0.0;  // 脸部过小，无法评估

    // 定义左右对称点对
    // 涵盖眉毛、眼睛、鼻子、嘴唇、下颌中部
    std::pair<int,int> pairs[] = {
        // 眉毛
        {17, 26},   // 眉尾
        {18, 25},
        {19, 24},   // 眉中部
        {20, 23},
        {21, 22},   // 眉头
        // 眼睛
        {36, 45},   // 外眼角
        {37, 44},   // 下眼睑
        {38, 43},   // 上眼睑
        {39, 42},   // 内眼角
        // 鼻子
        {31, 35},   // 鼻翼
        // 嘴唇
        {48, 54},   // 嘴角
        {49, 53},   // 上唇外缘
        {50, 52},   // 上唇中点旁
        // 下颌（选少数稳定点，避免与中线计算重复）
        {3, 13},    // 下巴两侧
        {4, 12},
        {5, 11}
    };

    const int pairCount = sizeof(pairs) / sizeof(pairs[0]);
    double totalErr = 0.0;

    for (int i = 0; i < pairCount; ++i) {
        int l = pairs[i].first;
        int r = pairs[i].second;

        // X 轴对称误差：左右到中线的水平距离差
        double errX = std::abs((midX - lm[l].x) - (lm[r].x - midX));
        // Y 轴对称误差：左右高度差
        double errY = std::abs(lm[l].y - lm[r].y);

        // 合并误差
        totalErr += errX + errY;
    }

    // 归一化误差
    // 平均每对点的相对误差
    double avgRelativeErr = (totalErr / pairCount) / ipd;
    // 转换为百分比
    double percent = avgRelativeErr * 100.0;

    //  评分函数
    double score = 20.0 * std::exp(-std::pow(percent / 8.0, 2));
    // 保证不低于0
    if (score < 0.0) score = 0.0;
    return score;
    }




    double BeautyScorer::calcSkin(const cv::Mat& face,
                                  const std::vector<cv::Point>& landmarks68)
    {
        if (landmarks68.size() < 68) return 0.0;

        // 生成皮肤区域掩膜
        cv::Mat skinMask = cv::Mat::zeros(face.size(), CV_8UC1);
        // 构建面部核心区域的多边形，避开眼睛鼻子嘴
        std::vector<cv::Point> skinRegion = {
            landmarks68[0],   // 侧脸下缘
            landmarks68[4],   // 下巴
            landmarks68[8],   // 下巴底
            landmarks68[12],  // 另侧下巴
            landmarks68[16],  // 另侧侧脸
            landmarks68[26],  // 眉尾
            landmarks68[19],  // 眉中
            landmarks68[24],  // 另侧眉中
            landmarks68[17]   // 眉尾
        };
        cv::fillConvexPoly(skinMask, skinRegion, cv::Scalar(255));

        // 抠除眼部、鼻孔、嘴唇等高纹理区
        cv::fillConvexPoly(skinMask,
                           std::vector<cv::Point>{landmarks68[36], landmarks68[37], landmarks68[38],
                                                  landmarks68[39], landmarks68[40], landmarks68[41]},
                           cv::Scalar(0)); // 左眼
        cv::fillConvexPoly(skinMask,
                           std::vector<cv::Point>{landmarks68[42], landmarks68[43], landmarks68[44],
                                                  landmarks68[45], landmarks68[46], landmarks68[47]},
                           cv::Scalar(0)); // 右眼
        cv::fillConvexPoly(skinMask,
                           std::vector<cv::Point>{landmarks68[31], landmarks68[32], landmarks68[33],
                                                  landmarks68[34], landmarks68[35]},
                           cv::Scalar(0)); // 鼻子下缘
        cv::fillConvexPoly(skinMask,
                           std::vector<cv::Point>{landmarks68[48], landmarks68[60],
                                                  landmarks68[64], landmarks68[54]},
                           cv::Scalar(0)); // 嘴唇外圈

        // 计算皮肤区域的拉普拉斯方差，仅掩膜内有效像素
        cv::Mat gray;
        cv::cvtColor(face, gray, cv::COLOR_BGR2GRAY);
        cv::Mat laplacian;
        cv::Laplacian(gray, laplacian, CV_64F, 3);

        double sum = 0, sqSum = 0;
        int count = 0;
        for (int r = 0; r < laplacian.rows; ++r) {
            const double* row = laplacian.ptr<double>(r);
            const uchar* maskRow = skinMask.ptr<uchar>(r);
            for (int c = 0; c < laplacian.cols; ++c) {
                if (maskRow[c] > 0) {
                    double val = row[c];
                    sum += val;
                    sqSum += val * val;
                    count++;
                }
            }
        }
        if (count == 0) return 0.0;
        double mean = sum / count;
        double textureRaw = std::sqrt(sqSum / count - mean * mean);  // 标准差

        // 尺度归一化：除以平均瞳距的平方，消除图像大小影响
        double leftEyeX = (landmarks68[36].x + landmarks68[39].x) / 2.0;
        double rightEyeX = (landmarks68[42].x + landmarks68[45].x) / 2.0;
        double eyeDist = std::abs(rightEyeX - leftEyeX);
        if (eyeDist < 1e-6) return 0.0;
        double texture = textureRaw / (eyeDist * eyeDist);  // 归一化纹理强度

        // 非线性评分
        double idealLow  = 0.0013;
        double idealHigh = 0.0015;
        double score;
        if (texture < idealLow) {
            // 过于平滑（可能磨皮或失焦），缓慢扣分
            score = 20.0 * (texture / idealLow);
        } else if (texture <= idealHigh) {
            score = 20.0;
        } else {
            // 纹理粗糙，超出上限后衰减，到 texture=0.005 时归零
            double maxAccept = 0.005;
            if (texture >= maxAccept) score = 0.0;
            else score = 20.0 * (1.0 - (texture - idealHigh) / (maxAccept - idealHigh));
        }
        return std::max(0.0, std::min(score, 20.0));
    }



        double BeautyScorer::calcFeatureProportions(const std::vector<cv::Point>& lm) {
            if (lm.size() < 68) return 0.0;

            // 平滑非线性评分函数：高斯曲线，越接近理想值分数越高，下降柔和
            auto smoothScore = [](double diff, double ideal, double sigma = 0.4) -> double {
                double deviation = std::abs(diff - ideal);
                // 高斯非线性平滑，范围 [1.0 ~ 5.0]
                return 5.0 * exp(- (deviation * deviation) / (2 * sigma * sigma));
            };
            auto distX = [&](int i, int j) { return std::abs(lm[i].x - lm[j].x); };
            auto distY = [&](int i, int j) { return std::abs(lm[i].y - lm[j].y); };

            // 鼻部比例 (理想值 0.67)
            double noseWidth  = distX(31, 35);   // 鼻翼宽度
            double noseLength = distY(27, 30);   // 鼻尖到眉心高度
            double nasalIndex = (noseLength > 1e-6) ? (noseWidth / noseLength) : 0.0;
            double nasalScore = smoothScore(nasalIndex, 0.67);

            // 唇部比例 (理想值：下唇宽/嘴宽=0.70)
            double mouthWidth    = distX(48, 54); // 嘴宽
            double lowerLipWidth = distX(53, 55); // 下唇宽度
            double lipRatio      = (mouthWidth > 1e-6) ? (lowerLipWidth / mouthWidth) : 0.0;
            double lipScore      = smoothScore(lipRatio, 0.70);

            // 眼部比例 (理想值：眼高/眼宽=0.40)
            double leftEyeWidth  = distX(36, 39); // 左眼宽度
            double leftEyeHeight = distY(37, 41); // 左眼高度
            double rightEyeWidth = distX(42, 45); // 右眼宽度
            double rightEyeHeight= distY(43, 47); // 右眼高度
            double avgEyeWidth   = (leftEyeWidth + rightEyeWidth) / 2.0;
            double avgEyeHeight  = (leftEyeHeight + rightEyeHeight) / 2.0;
            double eyeAspectRatio= (avgEyeWidth > 1e-6) ? (avgEyeHeight / avgEyeWidth) : 0.0;
            double eyeShapeScore = smoothScore(eyeAspectRatio, 0.40);


            // 面部宽度比例 (理想值：下颌宽/颧宽=0.68)
            double bizygomatic = distX(1, 15);  // 颧骨宽度
            double bigonial    = distX(5, 11);  // 下颌角宽度
            double widthRatio  = (bizygomatic > 1e-6) ? (bigonial / bizygomatic) : 0.0;
            double widthScore  = smoothScore(widthRatio, 0.68);

            // 计算总分
            double total = (nasalScore + lipScore + eyeShapeScore +  widthScore);
            return std::max(0.0, std::min(total, 20.0));
        }

double BeautyScorer::sanTingScore()    const { return m_sanTing; }
double BeautyScorer::wuYanScore()      const { return m_wuYan; }
double BeautyScorer::symmetryScore()   const { return m_symmetry; }
double BeautyScorer::skinScore()       const { return m_skin; }
double BeautyScorer::featureProportionsScore()    const { return m_featureProportions; }