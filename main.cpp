
#include "startwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    StartWindow w;  // 启动页 = 真正主窗口
    w.show();
    return a.exec();
}