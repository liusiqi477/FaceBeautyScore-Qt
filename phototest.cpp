#include "phototest.h"
#include "ui_phototest.h"
#include "startwindow.h"  // 【关键】cpp里才包含startwindow.h
#include <QFileDialog>
#include <QMessageBox>


// 【修正】构造函数语法
PhotoTest::PhotoTest(StartWindow *startWindow, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PhotoTest)
    , m_startWindow(startWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("颜值评分系统");
    ui->selectBtn->setText("选择图片");

    // ============ 初始化分数显示 ============
    // 你需要在UI里放一个 QWidget，名字叫 widget_score
    m_scoreDisplay = new ScoreDisplay(ui->widget_score);
}

PhotoTest::~PhotoTest()
{
    delete ui;
}

// 返回按钮
void PhotoTest::on_backBtn_clicked()
{
    if (m_startWindow) {
        m_startWindow->show();
        this->hide();

        // ======================
        // 【重置所有状态】
        // ======================
        currentImagePath.clear();           // 清空图片路径
        ui->label->clear();                 // 清空图片显示
        ui->detectBtn->setVisible(true);    // 检测按钮重新显示
        ui->selectBtn->setText("选择图片"); // 选择按钮恢复初始文字
        // 先清空分数
        m_scoreDisplay->clear();

    }
}



// 选择图片 + 显示正脸 按钮
// ==========================
void PhotoTest::on_selectBtn_clicked()
{
    // 状态 1：选择图片
    if (ui->selectBtn->text() == "选择图片") {
        currentImagePath = QFileDialog::getOpenFileName(this, "选择图片", "", "图片 (*.png *.jpg *.jpeg)");
        if (currentImagePath.isEmpty()) return;

        // 显示原图
        QPixmap pix(currentImagePath);
        ui->label->setPixmap(pix.scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->selectBtn->setText("显示正脸");
    }

    // 状态 2：显示【带68点的标准正脸】
    else if (ui->selectBtn->text() == "显示正脸") {
        cv::Mat aligned = detector.getAlignedFace();
        std::vector<cv::Point> points = detector.getAlignedLandmarks();

        if (aligned.empty() || points.size() < 68) {
            QMessageBox::warning(this, "提示", "请先点击【开始检测】！");
            return;
        }

        // 在正脸图上画68个点
        cv::Mat showFace = aligned.clone();
        for (auto &p : points) {
            cv::circle(showFace, p, 1, cv::Scalar(0, 255, 0), -1);
        }

        // 转成Qt图片并显示
        cv::cvtColor(showFace, showFace, cv::COLOR_BGR2RGB);
        QImage qimg(showFace.data, showFace.cols, showFace.rows, showFace.step, QImage::Format_RGB888);

        ui->label->setPixmap(QPixmap::fromImage(qimg).scaled(ui->label->size(), Qt::KeepAspectRatio));

        // 切回选择图片
        ui->selectBtn->setText("显示原图");
    }
    else if (ui->selectBtn->text() == "显示原图"){
        QPixmap pix(currentImagePath);
        ui->label->setPixmap(pix.scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->selectBtn->setText("显示正脸");
    }
}

// ==========================
// 开始检测（68点 + 对齐）
// ==========================
void PhotoTest::on_detectBtn_clicked()
{
    if (currentImagePath.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择图片！");
        return;
    }

    QImage result = detector.detect68Points(currentImagePath);
    if (result.isNull()) {
        QMessageBox::warning(this, "提示", "未检测到人脸，请更换图片！");
        m_scoreDisplay->clear();
        return;
    }

    ui->label->setPixmap(QPixmap::fromImage(result).scaled(ui->label->size(), Qt::KeepAspectRatio));

    // 人脸数量判断
    int faceCnt = detector.getFaceCount();
    if(faceCnt > 1){
        QMessageBox::information(this, "提示", "检测到多张人脸，已自动选取最大人脸评分");
    }

    // 取对齐后的关键点评分（保证比例准确）
    auto points = detector.getAlignedLandmarks();
    auto face = detector.getAlignedFace();

    // 空值防护
    if(points.size() < 68 || face.empty()){
        QMessageBox::warning(this, "错误", "人脸数据异常，无法评分");
        return;
    }

    double total = m_beauty.calculateTotalScore(points, face);

    // 显示分数
    m_scoreDisplay->updateScore(
        m_beauty.sanTingScore(),
        m_beauty.wuYanScore(),
        m_beauty.symmetryScore(),
        m_beauty.skinScore(),
        m_beauty.featureProportionsScore(),
        total
        );
    // ===== 五官分析 =====
    FacialFeatureAnalyzer analyzer;
    FacialFeatures feat = analyzer.analyze(points);   // ✅ 使用已有的 points（即对齐后的68点）
    m_scoreDisplay->setFacialFeatures(feat);
}