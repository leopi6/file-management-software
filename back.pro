QT += widgets
CONFIG += c++17

# 包含路径（当前不需要额外的包含路径）
INCLUDEPATH += $$PWD/libs/7zip

# 源文件
SOURCES += \
    main.cpp \
    widget.cpp

# 头文件
HEADERS += \
    widget.h

# UI 文件
FORMS += \
    widget.ui




QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
