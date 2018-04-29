#-------------------------------------------------
#
# Project created by QtCreator 2018-04-29T16:40:50
#
#-------------------------------------------------

QT       += core gui

TARGET = image
TEMPLATE = lib
CONFIG += staticlib
QMAKE_CXXFLAGS += /std:c++17
CONFIG += warn_off

# фиксики для кодировки в консоли
QMAKE_EXTRA_TARGETS += before_build makefilehook

makefilehook.target = $(MAKEFILE)
makefilehook.depends = .beforebuild

PRE_TARGETDEPS += .beforebuild

before_build.target = .beforebuild
before_build.depends = FORCE
before_build.commands = chcp 1251


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
        Image.cpp \
    Kernel.cpp \
    InterestPoints.cpp \
    ImageConverter.cpp \
    Pyramid.cpp \
    Descriptor.cpp \
    Ransac.cpp

HEADERS += \
        Image.h \
    Kernel.h \
    InterestPoints.h \
    ImageConverter.h \
    Pyramid.h \
    Descriptor.h \
    Ransac.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../linalg/release/ -llinalg
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../linalg/debug/ -llinalg

INCLUDEPATH += $$PWD/../linalg
DEPENDPATH += $$PWD/../linalg

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../linalg/release/liblinalg.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../linalg/debug/liblinalg.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../linalg/release/linalg.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../linalg/debug/linalg.lib
