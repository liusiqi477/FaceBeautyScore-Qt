#ifndef STARTWINDOW_H
#define STARTWINDOW_H

#include <QMainWindow>

// 【关键】前向声明 PhotoTest，避免循环依赖
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

private:
    Ui::StartWindow *ui;
    PhotoTest *photoTest;  // 只声明，不实例化
};

#endif // STARTWINDOW_H