#include "startwindow.h"
#include "ui_startwindow.h"
#include "phototest.h"  // 【关键】cpp里才包含phototest.h

StartWindow::StartWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::StartWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("人脸68点检测系统");
    photoTest = nullptr;
}

StartWindow::~StartWindow()
{
    delete ui;
    if (photoTest) delete photoTest;
}

// 点击照片测试
void StartWindow::on_photoTestBtn_clicked()
{
    if (!photoTest) {
        photoTest = new PhotoTest(this);  // 【关键】this是StartWindow*，类型匹配
    }
    photoTest->show();
    this->hide();
}