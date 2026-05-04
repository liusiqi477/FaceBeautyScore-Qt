QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    beautyscorer.cpp \
    facedetector.cpp \
    facialfeatureanalyzer.cpp \
    main.cpp \
    phototest.cpp \
    scoredisplay.cpp \
    startwindow.cpp \
    videotest.cpp

HEADERS += \
    beautyscorer.h \
    facedetector.h \
    facialfeatureanalyzer.h \
    phototest.h \
    scoredisplay.h \
    startwindow.h \
    videotest.h

FORMS += \
    phototest.ui \
    startwindow.ui \
    videotest.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
# 强制指定 MSVC 编译器
CONFIG += msvc

# 手动关联 Windows 10 SDK 头文件和库
INCLUDEPATH += "C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt"
INCLUDEPATH += "C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\shared"
INCLUDEPATH += "C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\um"

LIBS += -L"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0\ucrt\x64"
LIBS += -L"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0\um\x64"

# 关联 MSVC 工具集
INCLUDEPATH += "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.38.33130\include"
LIBS += -L"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.38.33130\lib\x64"

# 强制使用高版本 MSVC
CONFIG += msvc_version=1940

# 确保 Qt 用最新的 STL
CONFIG += c++20
DEFINES += _HAS_STD_BYTE=1



# OpenCV build 根目录（核心：用你的实际路径，斜杠是 /）
OPENCV_DIR = D:/download/opencv/opencv/build

# 头文件路径（指向 build 下的 include，这是 Qt 能识别的正确路径）
INCLUDEPATH += $$OPENCV_DIR/include
INCLUDEPATH += $$OPENCV_DIR/include/opencv2

# 库文件路径（你只有 vc16，所以指向 vc16/lib）
LIBS += -L$$OPENCV_DIR/x64/vc16/lib

# 链接 OpenCV 库（按你实际版本改数字！比如 4.8.0=480，4.7.0=470）
# Debug 模式（带 d）和 Release 模式（不带 d）区分
CONFIG(debug, debug|release) {
    LIBS += -lopencv_world4120d  # 替换 480 为你的 OpenCV 版本号
} else {
    LIBS += -lopencv_world4120    # 替换 480 为你的 OpenCV 版本号
}






# 必须加！否则 dlib 编译报错
QMAKE_CXXFLAGS += /bigobj

# 关闭 dlib 自带GUI，避免和 Qt 冲突
# DEFINES += DLIB_NO_GUI_SUPPORT
DEFINES += DLIB_STATIC_LINKING_ONLY

# C++17 以上（dlib 要求）
CONFIG += c++17


# # 假设 vcpkg 安装在 D:\download\vcpkg
# INCLUDEPATH += D:/download/vcpkg/installed/x64-windows/include
# LIBS += -LD:/download/vcpkg/installed/x64-windows/lib -ldlib


# # # 链接 vcpkg 提供的 BLAS/LAPACK 依赖
# LIBS += -lopenblas


# ==========================
# vcpkg dlib 自动配置
# ==========================
INCLUDEPATH += D:/download/vcpkg/installed/x64-windows/include
LIBS += -LD:/download/vcpkg/installed/x64-windows/lib
LIBS += -ldlib
# LIBS += -lopenblas
LIBS += -lopenblas