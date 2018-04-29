#-------------------------------------------------
#
# Project created by QtCreator 2018-04-29T16:41:27
#
#-------------------------------------------------

QT       -= core gui

TARGET = linalg
TEMPLATE = lib
CONFIG += staticlib
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
        linalg.cpp \
    alglibinternal.cpp \
    alglibmisc.cpp \
    ap.cpp \
    dataanalysis.cpp \
    diffequations.cpp \
    fasttransforms.cpp \
    integration.cpp \
    interpolation.cpp \
    linalg.cpp \
    optimization.cpp \
    solvers.cpp \
    specialfunctions.cpp \
    statistics.cpp

HEADERS += \
        linalg.h \
    alglibinternal.h \
    alglibmisc.h \
    ap.h \
    dataanalysis.h \
    diffequations.h \
    fasttransforms.h \
    integration.h \
    interpolation.h \
    linalg.h \
    optimization.h \
    solvers.h \
    specialfunctions.h \
    statistics.h \
    stdafx.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
