#-------------------------------------------------
#
# Project created by QtCreator 2018-04-29T16:00:38
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gui
TEMPLATE = app
QMAKE_CXXFLAGS += /std:c++17
CONFIG += warn_off

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        MainWindow.cpp

HEADERS += \
        MainWindow.h

FORMS += \
        MainWindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../image/release/ -limage
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../image/debug/ -limage

INCLUDEPATH += $$PWD/../image
DEPENDPATH += $$PWD/../image

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../image/release/ -limage
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../image/debug/ -limage

INCLUDEPATH += $$PWD/../image
DEPENDPATH += $$PWD/../image

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../image/release/libimage.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../image/debug/libimage.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../image/release/image.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../image/debug/image.lib