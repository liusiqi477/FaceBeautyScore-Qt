#ifndef STARTWINDOW_H
#define STARTWINDOW_H

#include <QMainWindow>
#include "videotest.h"
#include <QPainter>
#include <QRadialGradient>
#include <QtMath>         // for qSin, qCos, M_PI

// 前向声明 PhotoTest，避免循环依赖
class PhotoTest;

QT_BEGIN_NAMESPACE
namespace Ui { class StartWindow; }
QT_END_NAMESPACE

class StartWindow : public QMainWindow
{
    Q_OBJECT

public:
    StartWindow(QWidget *parent = nullptr);
    ~StartWindow();

private slots:
    void on_photoTestBtn_clicked();



    void on_videoTestBtn_clicked();

private:
    Ui::StartWindow *ui;
    PhotoTest *photoTest;
    VideoTest *videoTest;
};

#endif // STARTWINDOW_H